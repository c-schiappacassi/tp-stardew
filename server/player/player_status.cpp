#include "player_status.h"

#include <algorithm>
#include <cstddef>
#include <limits>

#include "game/time/day_cycle.h"

PlayerStatus::PlayerStatus(const ZoneDataSource& zones): zones(zones) {}

void PlayerStatus::fillPublicData(ServerStatus& status,const std::map<uint16_t, PlayerRecord>& players) const {
    status.players.clear();
    for (const auto& [id, record] : players) {
        if (!record.connected) {
            continue;
        }

        status.players.emplace(record.name, record.player.getPublicData());
    }
}

void PlayerStatus::completeLocalData(ServerStatus& status,const PlayerRecord& record,bool includeBroken,
                                     const DayCycle* day,bool multiplayer) const {
    status.playerId = record.id;
    status.localPlayer = record.player.getLocalData();
    zones.completeZoneData(record.player, status.localPlayer, includeBroken);
    status.community = record.community;
    status.community.lastDonation = record.lastDonation;
    status.gameCompleted = status.gameCompleted || status.community.completed;
    status.dayConfirmationRequired = !status.gameCompleted && day &&
        day->requiresConfirmation(record.id, record.player, multiplayer);
    if (status.gameCompleted) status.dayEnded = false;
    if (day) status.timeOfDayMinutes = day->timeOfDayMinutes();
}

ServerStatus PlayerStatus::buildForPlayer(const std::map<uint16_t, PlayerRecord>& players,const PlayerRecord& record,
                                         const DayCycle* day) const {
    ServerStatus status{};
    status.status = StatusCode::Ok;

    fillPublicData(status, players);
    completeLocalData(status, record, false, day, players.size() > 1);

    return status;
}

void PlayerStatus::publish(std::map<uint16_t, PlayerRecord>& players,const ServerStatus& base,const DayCycle* day) const {
    for (auto& [id, record] : players) {
        if (!record.connected) {
            continue;
        }
        ServerStatus status = base;
        fillPublicData(status, players);
        completeLocalData(status, record, true, day, players.size() > 1);

        const auto messageCount = std::min(record.chatMessages.size(),static_cast<std::size_t>(std::numeric_limits<uint16_t>::max()));
        const auto messagesEnd = record.chatMessages.begin() + static_cast<std::ptrdiff_t>(messageCount);
        status.localPlayer.newMessages.assign(record.chatMessages.begin(), messagesEnd);

        try {
            record.output->push(status);
            record.chatMessages.erase(record.chatMessages.begin(), messagesEnd);
        } catch (const ClosedQueue&) {
            
        }
    }
}
