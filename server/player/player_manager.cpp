#include "player_manager.h"

#include <utility>

PlayerManager::PlayerManager(const PlayerConfig& config, const MovementRules& movement,
        const ZoneDataSource& zones, PlayerInteractions& interactions, const PlayerSpawnSource& spawns,
        uint8_t maxPlayers, ChatServices chatServices):
        maxPlayers(maxPlayers),
        factory(config),
        updates(movement),
        playerStatus(zones),
        actions(interactions),
        chat(std::move(chatServices)),
        spawns(spawns),
        zones(zones),
        movement(movement),
        day(std::chrono::milliseconds{config.dayDurationMs}) {}


void PlayerManager::disconnectPlayer(uint16_t playerId) {
    const auto it = players.find(playerId);
    if (it != players.end()) {
        if (it->second.connected) chat.onDisconnect(playerId);
        it->second.connected = false;
        day.forgetConfirmation(playerId);
        it->second.output.reset();
        it->second.chatMessages.clear();
        it->second.lastDonation.reset();
        it->second.player.setMovement(Direction::None);
    }
}

void PlayerManager::closeOutputs() {
    for (auto& [id, record] : players) {
        if (!record.output) continue;
        try {
            record.output->close();
        } catch (const std::runtime_error&) {}
    }
}

std::vector<CollisionBounds> PlayerManager::getCollisionBounds(Zone zone) const {
    std::vector<CollisionBounds> result;
    for (const auto& [id, record] : players) {
        if (record.connected && record.player.getZone() == zone) {
            result.push_back(record.player.getCollisionBounds(record.player.getPosition()));
        }
    }
    return result;
}

void PlayerManager::updateToolStates(
    std::chrono::steady_clock::time_point now) {
    updates.updateToolStates(players, now);
}

void PlayerManager::advancePlayers(int normalStep) { 
    updates.advancePlayers(players, normalStep); 
}

void PlayerManager::publishStatus(const ServerStatus& base) {
    auto status = base;
    status.dayEnded = !base.gameCompleted && day.hasEnded();
    playerStatus.publish(players, status, &day);
}

void PlayerManager::handleCommand(const PlayerCommand& command, bool gameCompleted) {
    if (!command.command) {
        return;
    }
    if (command.command->type() == MessageType::ExitGame) {
        disconnectPlayer(command.playerId);
        return;
    }

    auto it = players.find(command.playerId);
    if (it == players.end() || !it->second.connected) {
        return;
    }

    if (command.command->type() == MessageType::Chat) {
        chat.handle(players, command.playerId, static_cast<const ChatDTO&>(*command.command).text,
                    day.hasEnded() || gameCompleted);
        return;
    }

    if (command.command->type() == MessageType::Donate) {
        chat.donate(it->second, static_cast<const DonateDTO&>(*command.command),
                    day.hasEnded() || gameCompleted);
        return;
    }

    if (gameCompleted) return;

    if (command.command->type() == MessageType::DayOk) {
        day.acknowledge(command.playerId, it->second.player, isMultiplayer());
        return;
    }

    if (command.command->type() == MessageType::DayCancel) {
        day.cancel(command.playerId, it->second.player);
        return;
    }

    if (!day.hasEnded()) {
        if (command.command->type() == MessageType::Interact &&
            chat.interact(it->second, static_cast<const InteractDTO&>(*command.command).dir, false)) {
            return;
        }
        actions.execute(it->second.player, *command.command, command.playerId);
    }
}

std::vector<Combatant> PlayerManager::getCombatants() {
    std::vector<Combatant> result;
    for (auto& [id, record]: players) {
        if (record.connected) {
            result.push_back(Combatant{id, &record.player});
        }
    }
    return result;
}

bool PlayerManager::advanceDay(std::chrono::milliseconds elapsed) {
    std::vector<DayCycle::Participant> participants;
    participants.reserve(players.size());
    for (auto& [id, record]: players) {
        participants.push_back({id, record.player, record.connected});
    }
    return day.advance(elapsed, participants, spawns);
}

bool PlayerManager::isDayEnded() const { return day.hasEnded(); }

std::chrono::milliseconds PlayerManager::remainingDayTime() const { return day.remainingTime(); }

bool PlayerManager::hasConnectedPlayers() const {
    for (const auto& [id, record]: players) {
        if (record.connected) return true;
    }
    return false;
}

void PlayerManager::collectCommunityRewards() {
    for (auto& [id, record] : players) chat.collectCommunityRewards(record, day.hasEnded());
}

void PlayerManager::finishGame() {
    for (auto& [id, record] : players) record.player.setMovement(Direction::None);
}
