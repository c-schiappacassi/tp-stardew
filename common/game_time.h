#pragma once

#include <chrono>
#include <cstdint>

namespace game_time {
inline constexpr auto DAY_DURATION = std::chrono::minutes{14};
inline constexpr uint16_t START_MINUTES = 6 * 60;
inline constexpr uint16_t DAY_MINUTES = 20 * 60;
inline constexpr uint16_t MINUTES_PER_24_HOURS = 24 * 60;
}
