#include "interior_furniture_position.h"

#include "entities/entity_dimensions.h"

static int furnitureSpriteWidth(FurnitureId type, Direction direction) {
    switch (type) {
        case FurnitureId::Chair:
        case FurnitureId::BlueJunimoPushee:
        case FurnitureId::GreenJunimoPushee:
        case FurnitureId::YellowJunimoPushee:
        case FurnitureId::PinkJunimoPushee:
        case FurnitureId::SimpleLamp:
        case FurnitureId::SquareLamp:
        case FurnitureId::GreenStool:
        case FurnitureId::Chest:
            return 16;
        case FurnitureId::BeigeRug:
            return 24;
        case FurnitureId::Tv:
            return 26;
        case FurnitureId::Table:
        case FurnitureId::Bed:
        case FurnitureId::BedSheet:
        case FurnitureId::Bonsai:
        case FurnitureId::PottedTree:
        case FurnitureId::BlueArmCouch:
        case FurnitureId::ClockFirePlace:
        case FurnitureId::CatFirePlace:
        case FurnitureId::SimpleFirePlace:
        case FurnitureId::BookShelf:
        case FurnitureId::RoundTable:
        case FurnitureId::BrownArmCouch:
            return 32;
        case FurnitureId::BlueRug:
            return 37;
        case FurnitureId::BrownSofa:
            return direction == Direction::Left || direction == Direction::Right ? 32 : 48;
    }
    return 0;
}

Position interiorFurnitureBaseFromTileOrigin(const Position& origin, FurnitureId type, Direction direction) {
    return Position{origin.getX() + furnitureSpriteWidth(type, direction) / 2,
        origin.getY() + entity_dimensions::TILE_SIZE};
}

Position interiorFurnitureTileOriginFromBase(const Position& base, FurnitureId type, Direction direction) {
    return Position{base.getX() - furnitureSpriteWidth(type, direction) / 2,
        base.getY() - entity_dimensions::TILE_SIZE};
}
