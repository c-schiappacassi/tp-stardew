#include "player_manager.h"

#include <stdexcept>
#include <utility>

ServerStatus PlayerManager::addPlayer(const std::string& name,const PlayerData& appearance,const std::shared_ptr<OutQueue>& output) {
    ServerStatus result{};
    result.status = StatusCode::Error;

    if (name.empty() || !output) {
        result.errorMsg = "Could not register the player";
        return result;
    }

    for (const auto& [id, record] : players) {
        if (record.name == name) {
            result.errorMsg = "A player with that name already exists";
            return result;
        }
    }

    if (players.size() >= maxPlayers) {
        result.errorMsg = "The game is full";
        return result;
    }

    const uint16_t playerId = nextPlayerId;
    auto player = factory.create(appearance);
    if (!spawns.spawnPlayer(player)) {
        result.errorMsg = "No position is available for the player";
        return result;
    }
    if (day.hasEnded()) {
        player.passOut();
    }
    auto it =players.emplace(playerId, PlayerRecord{playerId, name, output, true, std::move(player)}).first;
    ++nextPlayerId;

    chat.collectCommunityRewards(it->second, day.hasEnded());

    auto status = playerStatus.buildForPlayer(players, it->second, &day);
    status.dayEnded = !status.gameCompleted && day.hasEnded();
    return status;
}
ServerStatus PlayerManager::resumePlayer(const std::string& name,const std::shared_ptr<OutQueue>& output) {
    ServerStatus result{};
    result.status = StatusCode::Error;
    for (auto& [id, record] : players) {
        if (record.name != name) {
            continue;
        }
        if (record.connected || !output) {
            result.errorMsg = "You are already connected to this game";
            return result;
        }
        if (!movement.canMove(record.player, record.player.getPosition()) &&
            !spawns.spawnPlayer(record.player)) {
            result.errorMsg = "No position is available to resume the game";
            return result;
        }
        if (day.hasEnded()) {
            record.player.passOut();
        }
        day.forgetConfirmation(id);
        record.connected = true;
        record.output = output;
        record.player.setMovement(Direction::None);
        chat.collectCommunityRewards(record, day.hasEnded());
        auto status = playerStatus.buildForPlayer(players, record, &day);
        status.dayEnded = !status.gameCompleted && day.hasEnded();
        return status;
    }
    result.errorMsg = "You do not have a player in this game";
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

std::map<std::string, PlayerDataLocal> PlayerManager::getPersistencePlayers() const {
    std::map<std::string, PlayerDataLocal> result;
    for (const auto& [id, record] : players) {
        PlayerDataLocal data = record.player.getLocalData();
        zones.completeZoneData(record.player, data, true);
        result.emplace(record.name, std::move(data));
    }
    return result;
}

void PlayerManager::restorePlayers(const std::map<std::string, PlayerDataLocal>& saved) {
    if (!players.empty() || saved.size() > maxPlayers || saved.size() > UINT16_MAX) {
        throw std::runtime_error("Cannot restore player roster");
    }
    std::map<uint16_t, PlayerRecord> restored;
    uint16_t id = 1;
    for (const auto& [username, data] : saved) {
        if (username.empty()) throw std::runtime_error("Empty username in save");
        Player player = factory.create(static_cast<const PlayerData&>(data));
        player.restoreData(data);
        restored.emplace(id, PlayerRecord{id, username, nullptr, false, std::move(player)});
        ++id;
    }
    players = std::move(restored);
    nextPlayerId = id;
}
