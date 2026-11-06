#include "collision_box.h"

CollisionBox::CollisionBox(int width, int height): width(width), height(height) {}

CollisionBox::CollisionBox(entity_dimensions::Size size):CollisionBox(size.width, size.height) {}

bool CollisionBox::isValid() const {
    return width > 0 && height > 0;
}

CollisionBounds CollisionBox::at(const Position& base) const {
    const int64_t left = static_cast<int64_t>(base.getX()) - width / 2;
    const int64_t top = static_cast<int64_t>(base.getY()) - height;
    return {left, top, left + width - 1, static_cast<int64_t>(base.getY()) - 1};
}
