#pragma once

#include <optional>

#include "entities/collision_box.h"
#include "position.h"
#include "protocol_types.h"

Position interiorFurnitureBaseFromTileOrigin(const Position& origin, FurnitureId type, Direction direction);
Position interiorFurnitureTileOriginFromBase(const Position& base, FurnitureId type, Direction direction);
std::optional<CollisionBox> interiorFurnitureCollisionBox(FurnitureId type, Direction direction);

CollisionBox furnitureCollisionBox(FurnitureId type, Direction direction);
