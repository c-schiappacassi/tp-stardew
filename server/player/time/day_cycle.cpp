#include "day_cycle.h"

#include "server_constants.h"

#include <algorithm>
#include <stdexcept>
#include <utility>
#include <vector>

#include "player/player.h"
#include "player/ports/player_spawn_source.h"

#include "game_time.h"

DayCycle::DayCycle(std::chrono::milliseconds duration): duration(duration) {
    if (duration.count() <= 0) {
        throw std::invalid_argument(server_constants::DAY_DURATION_MUST_BE_POSITIVE);
    }
}

bool DayCycle::hasEnded() const { return ended; }

std::chrono::milliseconds DayCycle::remainingTime() const { return duration - elapsed; }

uint16_t DayCycle::timeOfDayMinutes() const {
    const auto minutes =
            game_time::START_MINUTES + elapsed.count() * game_time::DAY_MINUTES / duration.count();
    return static_cast<uint16_t>(minutes % game_time::MINUTES_PER_24_HOURS);
}

bool DayCycle::requiresConfirmation(uint16_t playerId, const Player& player,bool multiplayer) const {
    return (player.isSleeping() || (!multiplayer && ended && player.isResting())) &&!confirmations.contains(playerId);
}

void DayCycle::acknowledge(uint16_t playerId, const Player& player, bool multiplayer) {
    if (requiresConfirmation(playerId, player, multiplayer)) {
        confirmations.insert(playerId);
    }
}

bool DayCycle::cancel(uint16_t playerId, Player& player, const PlayerSpawnSource& spawns) {
    if (ended || confirmations.contains(playerId) || !player.isSleeping())
        return false;
    const auto destination = spawns.recoveryFor(player.snapshot(), playerId);
    auto prepared = player;
    if (!destination || !prepared.cancelSleep() ||
        !prepared.restoreLocation(destination->zone, destination->level, destination->position, destination->interior))
        return false;
    player = std::move(prepared);
    return true;
}

void DayCycle::forgetConfirmation(uint16_t playerId) { confirmations.erase(playerId); }

bool DayCycle::advance(std::chrono::milliseconds time, std::span<const Participant> players,
                       const PlayerSpawnSource& spawns) {
    if (time.count() < 0) {
        throw std::invalid_argument(server_constants::TIME_CANNOT_MOVE_BACKWARDS);
    }
    bool connected = false;
    bool allIncapacitated = true;
    for (const auto& participant: players) {
        if (participant.connected) {
            connected = true;
            allIncapacitated &=participant.player.isResting() &&(!participant.player.isSleeping() || confirmations.contains(participant.id));
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
        if (participant.connected &&
            requiresConfirmation(participant.id, participant.player, players.size() > 1)) {
            return false;
        }
    }

    std::vector<Player> prepared;
    prepared.reserve(players.size());
    for (const auto& participant: players) {
        prepared.push_back(participant.player);
        const auto destination = spawns.recoveryFor(participant.player.snapshot(), participant.id);
        prepared.back().wakeUp();
        
        if (!destination ||!prepared.back().restoreLocation(destination->zone, destination->level,destination->position, destination->interior)) {
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
