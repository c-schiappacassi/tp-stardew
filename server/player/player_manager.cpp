#include "player_manager.h"

PlayerManager::PlayerManager(const PlayerConfig& config,const MovementRules& movement,const ZoneDataSource& zones,PlayerInteractions& interactions,uint8_t maxPlayers):
    maxPlayers(maxPlayers),
    factory(config),
    updates(movement),
    playerStatus(zones),
    actions(interactions) {}

    
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
    auto it = players.emplace(playerId, PlayerRecord{playerId, name, output, true, factory.create(appearance)}).first;
    ++nextPlayerId;

    return playerStatus.buildForPlayer(players, it->second);
}
void PlayerManager::disconnectPlayer(uint16_t playerId) {
    const auto it = players.find(playerId);
    if (it != players.end()) {
        it->second.connected = false;
        it->second.output.reset();
        it->second.player.setMovement(Direction::None);
    }
}

ServerStatus PlayerManager::resumePlayer(const std::string& name,
                                        const std::shared_ptr<OutQueue>& output) {
    ServerStatus result{};
    result.status = StatusCode::Error;
    for (auto& [id, record] : players) {
        if (record.name != name) continue;
        if (record.connected || !output) {
            result.errorMsg = "Ya estas conectado a esta partida";
            return result;
        }
        record.connected = true;
        record.output = output;
        record.player.setMovement(Direction::None);
        return playerStatus.buildForPlayer(players, record);
    }
    result.errorMsg = "No tenes un jugador en esta partida";
    return result;
}

std::optional<PlayerData> PlayerManager::dataFor(const std::string& name) const {
    for (const auto& [id, record] : players) {
        if (record.name == name) return record.player.getPublicData();
    }
    return std::nullopt;
}

bool PlayerManager::isMultiplayer() const { return players.size() > 1; }

void PlayerManager::updateToolStates(
    std::chrono::steady_clock::time_point now) {
    updates.updateToolStates(players, now);
}

void PlayerManager::advancePlayers(int normalStep) {
    updates.advancePlayers(players, normalStep);
}

void PlayerManager::publishStatus(const ServerStatus& base) const {
    playerStatus.publish(players, base);
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

    actions.execute(it->second.player, *command.command);
}
