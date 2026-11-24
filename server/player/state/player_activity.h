#pragma once

#include <chrono>
#include <cstdint>
#include <optional>

#include "dtos.h"

class PlayerActivity {
private:
    PlayerState state;
    Direction direction;
    Direction movementDirection = Direction::None;
    std::chrono::steady_clock::time_point nextActionTime{};
    std::optional<uint32_t> openChestId;

public:
    explicit PlayerActivity(const PlayerData& initialData);

    std::chrono::steady_clock::time_point actionReadyAt() const;
    PlayerState getState() const;
    Direction getDirection() const;
    bool canAct() const;
    bool isResting() const;
    bool isSleeping() const;
    bool cooldownEnded(std::chrono::steady_clock::time_point now) const;
    std::optional<uint32_t> getOpenChestId() const;

    void setMovement(Direction requested);
    void faceDirection(Direction requested);
    void stopMovementAfterRelocation();
    void resumeMovement();
    void startCooldown(std::chrono::steady_clock::time_point now,std::chrono::milliseconds interval);
    void startTool(Direction toolDirection);
    void updateToolState(std::chrono::steady_clock::time_point now);
    bool passOut();
    void sleep();
    bool cancelSleep();
    void wakeUp();
    void reset();
    void restoreData(const PlayerData& saved);
    void openChest(uint32_t id);
    void closeChest();
};
