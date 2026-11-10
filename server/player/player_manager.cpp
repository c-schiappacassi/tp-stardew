#include "player_manager.h"

PlayerManager::PlayerManager(const PlayerConfig& config,const MovementRules& movement,const ZoneDataSource& zones,PlayerInteractions& interactions,const PlayerSpawnSource& spawns,uint8_t maxPlayers):
        maxPlayers(maxPlayers),
        factory(config),
        updates(movement),
        playerStatus(zones),
        actions(interactions), spawns(spawns), movement(movement),dayDuration(config.dayDurationMs) {}


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
    if (dayEnded) {
        player.passOut();
    }
    auto it =players.emplace(playerId, PlayerRecord{playerId, name, output, true, std::move(player)}).first;
    ++nextPlayerId;

    auto status = playerStatus.buildForPlayer(players, it->second);
    status.dayEnded = dayEnded;
    return status;
}
void PlayerManager::disconnectPlayer(uint16_t playerId) {
    const auto it = players.find(playerId);
    if (it != players.end()) {
        it->second.connected = false;
        it->second.output.reset();
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
        record.connected = true;
        record.output = output;
        record.player.setMovement(Direction::None);
        auto status = playerStatus.buildForPlayer(players, record);
        status.dayEnded = dayEnded;
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

void PlayerManager::publishStatus(const ServerStatus& base) const {
    auto status = base;
    status.dayEnded = dayEnded;
    playerStatus.publish(players, status);
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

    if (command.command->type() == MessageType::DayOk) {
        const auto state = it->second.player.getPublicData().state;
        if (state == PlayerState::Unconscious || state == PlayerState::Sleeping) {
            it->second.readyForNextDay = true;
        }
        return;
    }

    actions.execute(it->second.player, *command.command, command.playerId);
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
    if (elapsed.count() < 0) {
        throw std::invalid_argument("El tiempo no puede retroceder");
    }
    bool connected = false;
    bool allIncapacitated = true;
    for (const auto& [id, record]: players) {
        if (!record.connected) {
            continue;
        }
        connected = true;
        if (record.player.canBeAttacked()) {
            allIncapacitated = false;
        }
    }
    if (!connected) {
        return false;
    }
    if (!dayEnded) {
        dayElapsed += elapsed;
        dayEnded = allIncapacitated || dayElapsed >= dayDuration;
        if (dayEnded) {
            for (auto& [id, record]: players) {
                record.player.passOut();
            }
        }
    }
    if (!dayEnded) {
        return false;
    }
    for (const auto& [id, record]: players) {
        if (record.connected && !record.readyForNextDay) {
            return false;
        }
    }
    auto prepared = players;
    for (auto& [id, record]: prepared) {
        record.player.wakeUp();
        if (!spawns.recoverPlayer(record.player)) {
            return false;
        }
        record.readyForNextDay = false;
    }
    players = std::move(prepared);
    dayElapsed = std::chrono::milliseconds{0};
    dayEnded = false;
    return true;
}
