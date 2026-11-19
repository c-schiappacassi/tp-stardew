#include "player_manager.h"

#include <utility>

PlayerManager::PlayerManager(const PlayerConfig& config,const MovementRules& movement,const ZoneDataSource& zones,PlayerInteractions& interactions,const PlayerSpawnSource& spawns,uint8_t maxPlayers,ChatServices chatServices):
        maxPlayers(maxPlayers),
        factory(config),
        updates(movement),
        playerStatus(zones),
        actions(interactions), chat(std::move(chatServices)), spawns(spawns), zones(zones), movement(movement),day(std::chrono::milliseconds{config.dayDurationMs}) {}


ServerStatus PlayerManager::addPlayer(const std::string& name,const PlayerData& appearance,const std::shared_ptr<OutQueue>& output) {
    ServerStatus result{};
    result.status = StatusCode::Error;

    if (name.empty() || !output) {
        result.errorMsg = "No se pudo registrar al jugador";
        return result;
    }

    for (const auto& [id, record] : players) {
        if (record.name == name) {
            result.errorMsg = "Ya existe un jugador con ese nombre";
            return result;
        }
    }

    if (players.size() >= maxPlayers) {
        result.errorMsg = "La partida esta llena";
        return result;
    }

    const uint16_t playerId = nextPlayerId;
    auto player = factory.create(appearance);
    if (combatBonusUnlocked) {
        player.applyCombatBonus();
    }
    if (!spawns.spawnPlayer(player)) {
        result.errorMsg = "No hay una posicion disponible para el jugador";
        return result;
    }
    if (day.hasEnded()) {
        player.passOut();
    }
    auto it =players.emplace(playerId, PlayerRecord{playerId, name, output, true, std::move(player)}).first;
    ++nextPlayerId;

    auto status = playerStatus.buildForPlayer(players, it->second, &day);
    status.dayEnded = day.hasEnded();
    return status;
}
void PlayerManager::disconnectPlayer(uint16_t playerId) {
    const auto it = players.find(playerId);
    if (it != players.end()) {
        if (it->second.connected) chat.onDisconnect(playerId);
        it->second.connected = false;
        day.forgetConfirmation(playerId);
        it->second.output.reset();
        it->second.chatMessages.clear();
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

ServerStatus PlayerManager::resumePlayer(const std::string& name,const std::shared_ptr<OutQueue>& output) {
    ServerStatus result{};
    result.status = StatusCode::Error;
    for (auto& [id, record] : players) {
        if (record.name != name) {
            continue;
        }
        if (record.connected || !output) {
            result.errorMsg = "Ya estas conectado a esta partida";
            return result;
        }
        if (!movement.canMove(record.player, record.player.getPosition()) &&
            !spawns.spawnPlayer(record.player)) {
            result.errorMsg = "No hay una posicion disponible para retomar la partida";
            return result;
        }
        if (day.hasEnded()) {
            record.player.passOut();
        }
        day.forgetConfirmation(id);
        record.connected = true;
        record.output = output;
        record.player.setMovement(Direction::None);
        auto status = playerStatus.buildForPlayer(players, record, &day);
        status.dayEnded = day.hasEnded();
        return status;
    }
    result.errorMsg = "No tenes un jugador en esta partida";
    return result;
}

std::optional<PlayerData> PlayerManager::dataFor(const std::string& name) const {
    for (const auto& [id, record] : players) {
        if (record.name == name) {
            return record.player.getPublicData();
        }
    }
    return std::nullopt;
}

std::map<std::string, PlayerDataLocal> PlayerManager::getPersistencePlayers() const {
    std::map<std::string, PlayerDataLocal> result;
    for (const auto& [id, record] : players) {
        PlayerDataLocal data = record.player.getLocalData();
        zones.completeZoneData(record.player, data, true);
        result.emplace(record.name, std::move(data));
    }
    return result;
}

bool PlayerManager::isMultiplayer() const { return players.size() > 1; }

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
    status.dayEnded = day.hasEnded();
    playerStatus.publish(players, status, &day);
}

void PlayerManager::handleCommand(const PlayerCommand& command) {
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
                    day.hasEnded());
        return;
    }

    if (command.command->type() == MessageType::DayOk) {
        day.acknowledge(command.playerId, it->second.player);
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

void PlayerManager::applyCombatBonus() {
    combatBonusUnlocked = true;
    for (auto& [id, record]: players) {
        record.player.applyCombatBonus();
    }
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
