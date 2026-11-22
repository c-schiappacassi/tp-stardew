#pragma once

#include <chrono>
#include <cstdint>
#include <set>
#include <span>

class Player;
class PlayerSpawnSource;

class DayCycle {
public:
    struct Participant {
        uint16_t id;
        Player& player;
        bool connected;
    };

private:
    std::chrono::milliseconds elapsed{0};
    std::chrono::milliseconds duration;
    bool ended = false;
    std::set<uint16_t> confirmations;

public:
    explicit DayCycle(std::chrono::milliseconds duration);
    bool hasEnded() const;
    std::chrono::milliseconds remainingTime() const;
    uint16_t timeOfDayMinutes() const;
    bool requiresConfirmation(uint16_t playerId, const Player& player) const;
    void acknowledge(uint16_t playerId, const Player& player);
    bool cancel(uint16_t playerId, Player& player);
    void forgetConfirmation(uint16_t playerId);
    bool advance(std::chrono::milliseconds time, std::span<const Participant> players,const PlayerSpawnSource& spawns);
};
