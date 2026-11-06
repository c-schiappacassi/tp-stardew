#include "game_clock.h"

#include <stdexcept>

GameClock::GameClock(std::chrono::steady_clock::time_point now): previous(now) {}

std::chrono::milliseconds GameClock::advance(std::chrono::steady_clock::time_point now) {
    if (now < previous) {
        throw std::invalid_argument("El reloj no puede retroceder");
    }
    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - previous);
    previous += elapsed;
    return elapsed;
}
