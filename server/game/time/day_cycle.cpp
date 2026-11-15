#include "day_cycle.h"

#include <algorithm>
#include <stdexcept>
#include <utility>
#include <vector>

#include "player/player.h"
#include "player/ports/player_spawn_source.h"

DayCycle::DayCycle(std::chrono::milliseconds duration): duration(duration) {
    if (duration.count() <= 0) {
        throw std::invalid_argument("La duracion del dia debe ser positiva");
    }
}

bool DayCycle::hasEnded() const { return ended; }

void DayCycle::acknowledge(uint16_t playerId, const Player& player) {
    if (player.isResting()) {
        confirmations.insert(playerId);
    }
}

void DayCycle::forgetConfirmation(uint16_t playerId) { confirmations.erase(playerId); }

bool DayCycle::advance(std::chrono::milliseconds time,std::span<const Participant> players,const PlayerSpawnSource& spawns) {
    if (time.count() < 0) {
        throw std::invalid_argument("El tiempo no puede retroceder");
    }
    bool connected = false;
    bool allIncapacitated = true;
    for (const auto& participant: players) {
        if (participant.connected) {
            connected = true;
            allIncapacitated &= !participant.player.canBeAttacked();
        }
    }
    if (!connected) {
        return false;
    }
    if (!ended) {
        elapsed += std::min(time, duration - elapsed);
        ended = allIncapacitated || elapsed == duration;
        if (ended) {
            for (const auto& participant: players) {
                participant.player.passOut();
            }
        }
    }
    if (!ended) {
        return false;
    }
    for (const auto& participant: players) {
        if (participant.connected && !confirmations.contains(participant.id)) {
            return false;
        }
    }

    std::vector<Player> prepared;
    prepared.reserve(players.size());
    for (const auto& participant: players) {
        prepared.push_back(participant.player);
        prepared.back().wakeUp();
        if (!spawns.recoverPlayer(prepared.back())) {
            return false;
        }
    }
    for (std::size_t i = 0; i < players.size(); ++i) {
        players[i].player = std::move(prepared[i]);
    }
    confirmations.clear();
    elapsed = std::chrono::milliseconds{0};
    ended = false;
    return true;
}
