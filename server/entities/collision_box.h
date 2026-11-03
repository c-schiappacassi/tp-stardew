#pragma once

#include <cstdint>

#include "position.h"

struct CollisionBounds {
    int64_t left;
    int64_t top;
    int64_t right;
    int64_t bottom;
};

struct CollisionBox {
    int offsetX;
    int offsetY;
    int width;
    int height;

    CollisionBounds at(const Position& position) const {
        const int64_t left = static_cast<int64_t>(position.getX()) + offsetX;
        const int64_t top = static_cast<int64_t>(position.getY()) + offsetY;
        return {left, top, left + width - 1, top + height - 1};
    }
};
