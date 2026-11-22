#include "player_manager.h"

#include <utility>

PlayerManager::PlayerManager(const PlayerConfig& config, const MovementRules& movement,
                             const ZoneDataSource& zones, PlayerInteractions& interactions,
                             const PlayerSpawnSource& spawns, uint8_t maxPlayers,
                             ChatServices chatServices):
        roster(config, movement, spawns, maxPlayers),
        day(std::chrono::milliseconds{config.dayDurationMs}),
        chat(*this, std::move(chatServices)),
        commands(*this, roster, day, chat, interactions),
        updates(movement),
        playerStatus(zones),
        spawns(spawns) {}

ServerStatus PlayerManager::addPlayer(const std::string& name, const PlayerData& appearance,const std::shared_ptr<OutQueue>& output) {
    return completeEntry(roster.addPlayer(name, appearance, output));
}

ServerStatus PlayerManager::resumePlayer(const std::string& name,const std::shared_ptr<OutQueue>& output) {
    return completeEntry(roster.resumePlayer(name, output));
}

ServerStatus PlayerManager::completeEntry(const PlayerRoster::Admission& admission) {
    if (!admission.record)
        return PlayerStatus::registrationFailure(admission.error);

    auto& record = *admission.record;
    if (day.hasEnded())
        record.getPlayer().passOut();
    day.forgetConfirmation(record.getId());
    chat.collectCommunityRewards(record, day.hasEnded());
    auto status = playerStatus.buildForPlayer(roster, record, &day);
    status.dayEnded = !status.gameCompleted && day.hasEnded();
    return status;
}

std::optional<PlayerData> PlayerManager::dataFor(const std::string& name) const {
    return roster.dataFor(name);
}

std::map<std::string, PlayerDataLocal> PlayerManager::getPersistencePlayers() const {
    return playerStatus.getPersistencePlayers(roster);
}

void PlayerManager::restorePlayers(const std::map<std::string, PlayerDataLocal>& saved) {
    roster.restorePlayers(saved);
}

bool PlayerManager::isMultiplayer() const { return roster.isMultiplayer(); }

std::vector<CollisionBounds> PlayerManager::getCollisionBounds(Zone zone) const {
    return roster.getCollisionBounds(zone);
}

std::vector<uint16_t> PlayerManager::connectedPlayerIds() const {
    return roster.connectedPlayerIds();
}

bool PlayerManager::advanceDay(std::chrono::milliseconds elapsed) {
    std::vector<DayCycle::Participant> participants;
    participants.reserve(roster.size());
    for (auto& [id, record]: roster)
        participants.push_back({id, record.getPlayer(), record.isConnected()});
    return day.advance(elapsed, participants, spawns);
}

bool PlayerManager::isDayEnded() const { return day.hasEnded(); }

std::chrono::milliseconds PlayerManager::remainingDayTime() const { return day.remainingTime(); }

bool PlayerManager::hasConnectedPlayers() const { return roster.hasConnectedPlayers(); }

void PlayerManager::collectCommunityRewards() {
    for (auto& [id, record]: roster) chat.collectCommunityRewards(record, day.hasEnded());
}

void PlayerManager::finishGame() {
    for (auto& [id, record]: roster) record.getPlayer().setMovement(Direction::None);
}

void PlayerManager::disconnectPlayer(uint16_t playerId) {
    auto* record = roster.find(playerId);
    if (!record)
        return;
    if (record->isConnected())
        chat.onDisconnect(playerId);
    day.forgetConfirmation(playerId);
    record->disconnect();
}

void PlayerManager::closeOutputs() {
    for (auto& [id, record]: roster) record.closeOutput();
}

void PlayerManager::updateToolStates(std::chrono::steady_clock::time_point now) {
    updates.updateToolStates(roster, now);
}

void PlayerManager::advancePlayers(int normalStep) { updates.advancePlayers(roster, normalStep); }

void PlayerManager::publishStatus(const ServerStatus& base) {
    auto status = base;
    status.dayEnded = !base.gameCompleted && day.hasEnded();
    playerStatus.publish(roster, status, &day);
}

void PlayerManager::handleCommand(const PlayerCommand& command, bool gameCompleted) {
    if (!command.command)
        return;
    if (command.command->type() == MessageType::ExitGame) {
        disconnectPlayer(command.playerId);
        return;
    }
    auto* record = roster.find(command.playerId);
    if (record && record->isConnected())
        commands.dispatch(*record, *command.command, gameCompleted);
}

PlayerSnapshot PlayerManager::snapshot(uint16_t playerId) const {
    return roster.require(playerId).getPlayer().snapshot();
}

std::map<uint8_t, Stack> PlayerManager::inventory(uint16_t playerId) const {
    return roster.require(playerId).getPlayer().getInventoryItems();
}

PlayerActionResult PlayerManager::apply(uint16_t playerId, const PlayerAction& action) {
    return commands.apply(playerId, action);
}

std::vector<Combatant> PlayerManager::combatants() const { return roster.combatants(); }
