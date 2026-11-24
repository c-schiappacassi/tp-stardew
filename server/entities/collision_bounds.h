#pragma once

#include <cstdint>
#include "position.h"

class CollisionBounds {
private:
    int64_t left;
    int64_t top;
    int64_t right;
    int64_t bottom;

public:
    CollisionBounds(int64_t left, int64_t top, int64_t right, int64_t bottom);

    int64_t getLeft() const;
    int64_t getTop() const;
    int64_t getRight() const;
    int64_t getBottom() const;

    bool isValid() const;
    bool intersects(const CollisionBounds& other) const;
    Position center() const;
};
