#include "day_cycle.h"

#include <algorithm>
#include <stdexcept>
#include <utility>
#include <vector>

#include "player/player.h"
#include "player/ports/player_spawn_source.h"
#include "game_time.h"

DayCycle::DayCycle(std::chrono::milliseconds duration): duration(duration) {
    if (duration.count() <= 0) {
        throw std::invalid_argument("Day duration must be positive");
    }
}

bool DayCycle::hasEnded() const { return ended; }

std::chrono::milliseconds DayCycle::remainingTime() const { return duration - elapsed; }

uint16_t DayCycle::timeOfDayMinutes() const {
    const auto minutes = game_time::START_MINUTES + elapsed.count() * game_time::DAY_MINUTES / duration.count();
    return static_cast<uint16_t>(minutes % game_time::MINUTES_PER_24_HOURS);
}

bool DayCycle::requiresConfirmation(uint16_t playerId, const Player& player) const {
    return (player.isSleeping() || (ended && player.isResting())) && !confirmations.contains(playerId);
}

void DayCycle::acknowledge(uint16_t playerId, const Player& player) {
    if (requiresConfirmation(playerId, player)) {
        confirmations.insert(playerId);
    }
}

bool DayCycle::cancel(uint16_t playerId, Player& player) {
    return !ended && !confirmations.contains(playerId) && player.cancelSleep();
}

void DayCycle::forgetConfirmation(uint16_t playerId) { confirmations.erase(playerId); }

bool DayCycle::advance(std::chrono::milliseconds time,std::span<const Participant> players,const PlayerSpawnSource& spawns) {
    if (time.count() < 0) {
        throw std::invalid_argument("Time cannot move backwards");
    }
    bool connected = false;
    bool allIncapacitated = true;
    for (const auto& participant: players) {
        if (participant.connected) {
            connected = true;
            allIncapacitated &= participant.player.isResting() &&
                (!participant.player.isSleeping() || confirmations.contains(participant.id));
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
        if (participant.connected && requiresConfirmation(participant.id, participant.player)) {
            return false;
        }
    }

    std::vector<Player> prepared;
    prepared.reserve(players.size());
    for (const auto& participant: players) {
        prepared.push_back(participant.player);
        prepared.back().wakeUp();
        if (!spawns.recoverPlayer(prepared.back(), participant.id)) {
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
