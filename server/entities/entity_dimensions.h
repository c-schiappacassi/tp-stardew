#pragma once

namespace entity_dimensions {
struct Size {
    int width;
    int height;
};

inline constexpr int TILE_SIZE = 16;

inline constexpr Size PLAYER{8, 8}; // player Foot
inline constexpr Size PLAYER_HITBOX{8, 24};
inline constexpr Size COW{24, 12};
inline constexpr Size CHICKEN{12, 8};
inline constexpr Size MINERAL{16, 16};
inline constexpr Size CROP{16, 16};
inline constexpr Size DECORATION{16, 16};
}
