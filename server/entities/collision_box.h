#pragma once

#include "collision_bounds.h"
#include "entities/entity_dimensions.h"
#include "position.h"

class CollisionBox {
private:
    int width;
    int height;

public:
    CollisionBox(int width, int height);
    explicit CollisionBox(entity_dimensions::Size size);

    bool isValid() const;
    CollisionBounds at(const Position& base) const;
};
