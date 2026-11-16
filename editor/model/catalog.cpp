#include "catalog.h"

#include <stdexcept>

namespace {
const int TILE_PIXELS = 16;

const std::vector<FloorInfo>& floors() {
    static const std::vector<FloorInfo> list{
        {TileTextureId::Grass, "Grass", "enviroment/outdoors_spring.png", 0, 112},
        {TileTextureId::Dirt, "Dirt", "enviroment/outdoors_spring.png", 16, 112},
        {TileTextureId::ConnctionStonePathGrassCorner, "Path (corner)", "market/world/town_spring.png", 0, 304},
        {TileTextureId::ConnctionStonePathGrassLeftSide, "Path (left side)", "market/world/town_spring.png", 0, 320},
        {TileTextureId::ConnctionStonePathGrassTop, "Path (top)", "market/world/town_spring.png", 16, 304},
        {TileTextureId::ConnctionStonePathGrassBottom, "Path (bottom)", "market/world/town_spring.png", 16, 336},
    };
    return list;
}

int tilesFor(int pixels) {
    return (pixels + TILE_PIXELS - 1) / TILE_PIXELS;
}

ElementInfo make(ElementId element, const std::string& name, const std::string& sheet, int x, int y, int width, int height) {
    const int tilesWidth = tilesFor(width);
    const int tilesHeight = tilesFor(height);
    return {element, name, sheet, x, y, width, height, tilesWidth, tilesHeight, {0, tilesHeight - 1, tilesWidth, 1}};
}

ElementInfo building(BuildingId id, const std::string& name, const std::string& sheet, int x, int y, int width, int height) {
    ElementInfo info = make(buildingElement(id), name, sheet, x, y, width, height);
    info.footprint = {0, 0, info.tilesWidth, info.tilesHeight};
    return info;
}

ElementInfo portal(PortalId id, const std::string& name, const std::string& sheet, int x, int y, int width, int height) {
    ElementInfo info = make(portalElement(id), name, sheet, x, y, width, height);
    info.footprint = {0, 0, info.tilesWidth, info.tilesHeight};
    return info;
}

ElementInfo tree(DecorId id, const std::string& name, const std::string& sheet, int x, int y, int width, int height) {
    ElementInfo info = make(decorElement(id), name, sheet, x, y, width, height);
    info.footprint = {info.tilesWidth / 2, info.tilesHeight - 1, 1, 1};
    return info;
}

ElementInfo decor(DecorId id, const std::string& name, const std::string& sheet, int x, int y, int width, int height) {
    return make(decorElement(id), name, sheet, x, y, width, height);
}

const std::vector<ElementInfo>& elements() {
    static const std::vector<ElementInfo> list{
        building(BuildingId::FarmHouse, "Farm house", "farm/environment/farm_buildings.png", 157, 268, 144, 145),
        building(BuildingId::StoneCabin, "Stone cabin", "farm/environment/cabins.png", 80, 336, 80, 112),
        building(BuildingId::WoodCabin, "Wood cabin", "farm/environment/cabins.png", 80, 0, 80, 112),
        building(BuildingId::BrickCabin, "Brick cabin", "farm/environment/cabins.png", 80, 112, 80, 112),
        building(BuildingId::MarniesShop, "Marnie's ranch", "market/world/buildings.png", 291, 0, 128, 112),
        building(BuildingId::PierresShop, "Pierre's general store", "market/world/buildings.png", 0, 0, 161, 160),
        building(BuildingId::RobinsShop, "Robin's carpenter shop", "market/world/buildings.png", 192, 127, 157, 193),
        building(BuildingId::ClintsShop, "Clint's blacksmith", "market/world/buildings.png", 161, 0, 130, 127),
        building(BuildingId::ComunityCenter, "Community center", "market/world/buildings.png", 0, 160, 192, 160),

        tree(DecorId::MapleTree, "Maple tree", "enviroment/trees.png", 0, 0, 48, 97),
        tree(DecorId::PineTree, "Pine tree", "enviroment/trees.png", 0, 320, 48, 97),
        tree(DecorId::MushroomTree, "Mushroom tree", "enviroment/mushroom.png", 0, 14, 48, 80),
        tree(DecorId::PinkTree, "Pink tree", "enviroment/trees.png", 292, 160, 48, 97),
        tree(DecorId::YellowTree, "Yellow tree", "enviroment/trees.png", 292, 480, 48, 97),
        tree(DecorId::BlueTree, "Blue tree", "enviroment/trees.png", 438, 1120, 48, 97),
        decor(DecorId::PinkFlowerGrass, "Pink flower grass", "enviroment/grass.png", 0, 0, 15, 20),
        decor(DecorId::OrangeFlowerGrass, "Orange flower grass", "enviroment/grass.png", 0, 20, 15, 20),
        decor(DecorId::OrangeGrass, "Orange grass", "enviroment/grass.png", 0, 40, 15, 20),
        decor(DecorId::VelvetGrass, "Velvet grass", "enviroment/grass.png", 15, 100, 15, 20),
        decor(DecorId::BlueGrass, "Blue grass", "enviroment/grass.png", 15, 160, 15, 20),
        decor(DecorId::PlainGrass, "Plain grass", "enviroment/grass.png", 0, 60, 15, 20),
        decor(DecorId::RedGrass, "Red grass", "enviroment/grass.png", 15, 40, 15, 20),
        decor(DecorId::SimpleBush, "Bush", "enviroment/bushes.png", 0, 9, 32, 36),
        decor(DecorId::RedBBerryBush, "Red berry bush", "enviroment/bushes.png", 32, 9, 32, 36),
        decor(DecorId::PinkBush, "Pink bush", "enviroment/bushes.png", 32, 95, 32, 32),
        decor(DecorId::PurpleBerryBush, "Purple berry bush", "enviroment/bushes.png", 32, 57, 32, 36),
        decor(DecorId::BlueBush, "Blue bush", "enviroment/bushes.png", 64, 57, 32, 36),
        decor(DecorId::BigRedBush, "Big red bush", "enviroment/bushes.png", 48, 132, 48, 43),
        decor(DecorId::StreatLamp, "Street lamp", "market/world/town_spring.png", 176, 448, 16, 64),
        decor(DecorId::FenceFront, "Fence (front)", "market/world/town_spring.png", 16, 449, 16, 32),
        decor(DecorId::FenceBack, "Fence (back)", "market/world/town_spring.png", 16, 400, 16, 32),
        decor(DecorId::PinkFlower, "Pink flower", "enviroment/outdoors_spring.png", 112, 160, 16, 16),
        decor(DecorId::BlueFlower, "Blue flower", "enviroment/outdoors_spring.png", 112, 192, 16, 16),
        decor(DecorId::WeirdGreenPlant, "Green plant", "caverns/plants.png", 0, 0, 16, 24),
        decor(DecorId::SmallFern, "Small fern", "caverns/plants.png", 16, 0, 16, 24),
        decor(DecorId::BigFern, "Big fern", "caverns/plants.png", 32, 0, 16, 24),
        decor(DecorId::PinkMushrom, "Pink mushroom", "caverns/plants.png", 96, 0, 16, 24),
        decor(DecorId::YellowMushrom, "Yellow mushroom", "caverns/plants.png", 128, 0, 16, 24),
        decor(DecorId::BabyRedMushrom, "Small red mushroom", "enviroment/mushroom.png", 0, 128, 16, 16),
        decor(DecorId::BigRedMushroom, "Big red mushroom", "enviroment/mushroom.png", 0, 102, 16, 25),
        decor(DecorId::DungeonPilar, "Pillar", "caverns/wall_tiles.png", 0, 384, 16, 48),
        decor(DecorId::DungeonTorch, "Torch", "caverns/wall_tiles.png", 64, 388, 16, 27),
        decor(DecorId::DungeonChain, "Chain", "caverns/wall_tiles.png", 64, 416, 16, 32),
        decor(DecorId::DungeonWindow, "Window", "caverns/wall_tiles.png", 64, 448, 16, 32),
        decor(DecorId::DungeonWallCrack, "Wall crack", "caverns/wall_tiles.png", 64, 480, 16, 32),
        decor(DecorId::SpiderWeb, "Spider web", "enviroment/grass.png", 15, 140, 16, 20),
        decor(DecorId::CornerSpiderWeb, "Corner spider web", "enviroment/grass.png", 31, 140, 16, 20),
        decor(DecorId::PottedPlants, "Potted plants", "market/world/town_spring.png", 368, 476, 47, 36),
        decor(DecorId::SmallBlueFlower, "Small blue flower", "market/world/town_spring.png", 353, 432, 16, 16),

        portal(PortalId::FarmToMine, "Mine entrance", "enviroment/outdoors_spring.png", 608, 160, 48, 48),
        portal(PortalId::ToNextLevel, "Ladder to next level", "caverns/level_portals.png", 0, 0, 16, 16),
        portal(PortalId::MineToFarm, "Mine exit", "caverns/level_portals.png", 0, 16, 16, 64),
    };
    return list;
}

}

const FloorInfo& FloorCatalog::infoOf(TileTextureId floor) {
    for (const FloorInfo& info: floors()) {
        if (info.floor == floor) {
            return info;
        }
    }
    throw std::out_of_range("piso no en el catalogo");
}

const std::vector<FloorInfo>& FloorCatalog::all() { return floors(); }


ElementId buildingElement(BuildingId id) { return {ElementKind::Building, static_cast<int>(id)}; }
ElementId decorElement(DecorId id) { return {ElementKind::Decor, static_cast<int>(id)}; }
ElementId portalElement(PortalId id) { return {ElementKind::Portal, static_cast<int>(id)}; }

const ElementInfo& ElementCatalog::infoOf(ElementId element) {
    for (const ElementInfo& info: elements()) {
        if (info.element == element) {
            return info;
        }
    }
    throw std::out_of_range("elemento no en el catalogo");
}

const std::vector<ElementInfo>& ElementCatalog::all() { return elements(); }