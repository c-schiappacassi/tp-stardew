#pragma once

#include <chrono>

class GameClock {
private:
    std::chrono::steady_clock::time_point previous;

public:
    explicit GameClock(std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now());
    std::chrono::milliseconds advance(std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now());
};
