#include "player_status.h"

PlayerStatus::PlayerStatus(const ZoneDataSource& zones): zones(zones) {}

void PlayerStatus::fillPublicData(ServerStatus& status,const std::map<uint16_t, PlayerRecord>& players,const PlayerRecord& recipient) const {
    status.players.clear();
    const Player& viewer = recipient.player;
    for (const auto& [id, record] : players) {
        if (!record.connected) {
            continue;
        }

        const Player& player = record.player;
        if (player.getZone() != viewer.getZone()) {
            continue;
        }
        if (viewer.getZone() == Zone::Dungeon &&
            player.getDungeonLevel() != viewer.getDungeonLevel()) {
            continue;
        }

        status.players.emplace(record.name, player.getPublicData());
    }
}

void PlayerStatus::completeLocalData(ServerStatus& status,const PlayerRecord& record,bool includeBroken) const {
    status.playerId = record.id;
    status.localPlayer = record.player.getLocalData();
    zones.completeZoneData(record.player, status.localPlayer, includeBroken);
}

ServerStatus PlayerStatus::buildForPlayer(const std::map<uint16_t, PlayerRecord>& players,const PlayerRecord& record) const {
    ServerStatus status{};
    status.status = StatusCode::Ok;

    fillPublicData(status, players, record);
    completeLocalData(status, record, false);

    return status;
}

void PlayerStatus::publish(const std::map<uint16_t, PlayerRecord>& players,const ServerStatus& base) const {
    ServerStatus status = base;

    for (const auto& [id, record] : players) {
        if (!record.connected) {
            continue;
        }
        fillPublicData(status, players, record);
        completeLocalData(status, record, true);

        try {
            record.output->push(status);
        } catch (const ClosedQueue&) {
            
        }
    }
}