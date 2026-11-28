#include "interior_furniture_position.h"

#include "entities/entity_dimensions.h"

namespace {

struct SpriteSize {
    int width;
    int height;
};

SpriteSize furnitureSpriteSize(FurnitureId type, Direction direction) {
    const bool sideways = direction == Direction::Left || direction == Direction::Right;
    switch (type) {
        case FurnitureId::Chair: return {16, 16};
        case FurnitureId::Table: return {32, 32};
        case FurnitureId::Bed:
        case FurnitureId::BedSheet: return {0, 0};
        case FurnitureId::Bonsai:
        case FurnitureId::PottedTree: return {28, 14};
        case FurnitureId::BlueArmCouch: return {32, 24};
        case FurnitureId::Tv: return {26, 26};
        case FurnitureId::BeigeRug: return {24, 20};
        case FurnitureId::ClockFirePlace: return {32, 20};
        case FurnitureId::BlueRug: return {37, 28};
        case FurnitureId::CatFirePlace: return {32, 20};
        case FurnitureId::SimpleFirePlace: return {32, 20};
        case FurnitureId::BlueJunimoPushee:
        case FurnitureId::GreenJunimoPushee:
        case FurnitureId::YellowJunimoPushee:
        case FurnitureId::PinkJunimoPushee: return {14, 12};
        case FurnitureId::SimpleLamp: return {16, 16};
        case FurnitureId::BookShelf: return {32, 16};
        case FurnitureId::SquareLamp: return {16, 16};
        case FurnitureId::RoundTable: return {32, 30};
        case FurnitureId::GreenStool: return {16,12};
        case FurnitureId::BrownSofa: return sideways ? SpriteSize{30, 32} : SpriteSize{48, 24};
        case FurnitureId::BrownArmCouch: return {32, 24};
        case FurnitureId::Chest: return {16, 16};
        case FurnitureId::PierreShopShelf: return sideways ? SpriteSize{32, 64} : SpriteSize{96, 26};
        case FurnitureId::PierreCashRegister: return {96, 16};
        case FurnitureId::PierreProductBin: return {30, 26};
        case FurnitureId::MarnieGrass: return {16, 16};
        case FurnitureId::CashRegister: return {48, 16};
        case FurnitureId::ClintsChair: return {16, 16};
        case FurnitureId::RobinCashRegister: return {48, 16};
        case FurnitureId::RobinPlantTable: return {14, 30};
    }
    return {0, 0};
}

}  // namespace

Position interiorFurnitureBaseFromTileOrigin(const Position& origin, FurnitureId type, Direction direction) {
    return Position{origin.getX() + furnitureSpriteSize(type, direction).width / 2,
        origin.getY() + entity_dimensions::TILE_SIZE};
}

Position interiorFurnitureTileOriginFromBase(const Position& base, FurnitureId type, Direction direction) {
    return Position{base.getX() - furnitureSpriteSize(type, direction).width / 2,
        base.getY() - entity_dimensions::TILE_SIZE};
}

CollisionBox furnitureCollisionBox(FurnitureId type, Direction direction) {
    const SpriteSize size = furnitureSpriteSize(type, direction);
    return CollisionBox{size.width, size.height};
}

std::optional<CollisionBox> interiorFurnitureCollisionBox(FurnitureId type, Direction direction) {
    switch (type) {
        case FurnitureId::BeigeRug:
        case FurnitureId::BlueRug:
        case FurnitureId::Bed:
        case FurnitureId::BedSheet:
            return std::nullopt;
        default:
            break;
    }
    const SpriteSize size = furnitureSpriteSize(type, direction);
    return CollisionBox{size.width, size.height};
}
