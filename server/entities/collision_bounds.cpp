#include "collision_bounds.h"

#include "server_constants.h"

#include <limits>
#include <numeric>
#include <stdexcept>

CollisionBounds::CollisionBounds(int64_t left, int64_t top, int64_t right, int64_t bottom):left(left), top(top), right(right), bottom(bottom) {}

int64_t CollisionBounds::getLeft() const { return left; }

int64_t CollisionBounds::getTop() const { return top; }

int64_t CollisionBounds::getRight() const { return right; }

int64_t CollisionBounds::getBottom() const { return bottom; }

bool CollisionBounds::isValid() const {
    return left <= right && top <= bottom;
}

bool CollisionBounds::intersects(const CollisionBounds& other) const {
    return isValid() && other.isValid() && left <= other.right && other.left <= right &&top <= other.bottom && other.top <= bottom;
}

Position CollisionBounds::center() const {
    if (!isValid()) {
        throw std::invalid_argument(server_constants::INVALID_COLLISION_BOUNDS);
    }
    const int64_t x = std::midpoint(right, left);
    const int64_t y = std::midpoint(bottom, top);
    if (x < std::numeric_limits<int>::min() || x > std::numeric_limits<int>::max() || y < std::numeric_limits<int>::min() || y > std::numeric_limits<int>::max()) {
        throw std::overflow_error(server_constants::COLLISION_CENTER_OUT_OF_RANGE);
    }
    return Position{static_cast<int>(x), static_cast<int>(y)};
}
