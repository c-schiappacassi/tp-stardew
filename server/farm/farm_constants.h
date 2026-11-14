#pragma once

#include <chrono>
#include <cstdint>

namespace farm_constants {
inline constexpr uint16_t INTERACTION_RANGE = 32;
inline constexpr uint16_t ACTION_ENERGY = 2;
inline constexpr std::chrono::milliseconds ACTION_INTERVAL{500};
}
