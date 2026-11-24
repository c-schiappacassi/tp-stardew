#include "player_status.h"

#include <utility>

#include "player/sessions/player_roster.h"
#include "player/time/day_cycle.h"

PlayerStatus::PlayerStatus(const ZoneDataSource& zones): zones(zones) {}

ServerStatus PlayerStatus::registrationFailure(const std::string& message) {
    ServerStatus status;
    status.status = StatusCode::Error;
    status.errorMsg = message;
    return status;
}

std::map<std::string, PlayerDataLocal> PlayerStatus::getPersistencePlayers(const PlayerRoster& players) const {
    std::map<std::string, PlayerDataLocal> result;
    for (const auto& [id, record]: players) {
        auto data = record.getPlayer().getLocalData();
        zones.completeZoneData(record.getPlayer().snapshot(), data, true);
        result.emplace(record.getName(), std::move(data));
    }
    return result;
}

ServerStatus PlayerStatus::buildForPlayer(const PlayerRoster& players, const PlayerRecord& record,const DayCycle* day) const {
    ServerStatus status{};
    status.status = StatusCode::Ok;

    fillPublicData(status, players);
    completeLocalData(status, record, false, day, players.size() > 1);

    return status;
}

void PlayerStatus::publish(PlayerRoster& players, const ServerStatus& base,const DayCycle* day) const {
    for (auto& [id, record]: players) {
        if (!record.isConnected()) {
            continue;
        }
        ServerStatus status = base;
        fillPublicData(status, players);
        completeLocalData(status, record, true, day, players.size() > 1);

        record.publishStatus(status);
    }
}

void PlayerStatus::fillPublicData(ServerStatus& status, const PlayerRoster& players) const {
    status.players.clear();
    for (const auto& [id, record]: players) {
        if (!record.isConnected()) {
            continue;
        }

        status.players.emplace(record.getName(), record.getPlayer().getPublicData());
    }
}

void PlayerStatus::completeLocalData(ServerStatus& status, const PlayerRecord& record,bool includeBroken, const DayCycle* day,bool multiplayer) const {
    status.playerId = record.getId();
    status.localPlayer = record.getPlayer().getLocalData();
    zones.completeZoneData(record.getPlayer().snapshot(), status.localPlayer, includeBroken);
    status.community = record.getCommunityData();
    status.gameCompleted = status.gameCompleted || status.community.completed;
    status.dayConfirmationRequired =!status.gameCompleted && day && day->requiresConfirmation(record.getId(), record.getPlayer(), multiplayer);
    if (status.gameCompleted)
        status.dayEnded = false;
    if (day)
        status.timeOfDayMinutes = day->timeOfDayMinutes();
}
