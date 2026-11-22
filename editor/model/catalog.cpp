#include "catalog.h"

#include <stdexcept>

namespace {
const int TILE_PIXELS = 16;
const char* const MINE = "caverns/mines_tiles.png";
const char* const OUTDOORS = "enviroment/outdoors_spring.png";
const char* const TOWN = "market/world/town_spring.png";

const std::vector<FloorInfo>& floors() {
    static const std::vector<FloorInfo> list{
        {TileTextureId::Grass, "Grass", OUTDOORS, 0, 112},
        {TileTextureId::Dirt, "Dirt", OUTDOORS, 16, 112},
        {TileTextureId::ConnctionStonePathGrassCorner, "Path (corner)", TOWN, 0, 304},
        {TileTextureId::ConnctionStonePathGrassLeftSide, "Path (left side)", TOWN, 0, 320},
        {TileTextureId::ConnctionStonePathGrassTop, "Path (top)", TOWN, 16, 304},
        {TileTextureId::ConnctionStonePathGrassBottom, "Path (bottom)", TOWN, 16, 336},
        {TileTextureId::StonePath, "Stone path", TOWN, 0, 368, 32, 32},

        {TileTextureId::DungeonFloor, "Floor", MINE, 1216, 704},
        {TileTextureId::DungeonVoid, "Void (black)", MINE, 1264, 640, 16, 16, false},
        {TileTextureId::DungeonLeftWall, "Left wall", MINE, 1088, 704, 16, 64, false, true},
        {TileTextureId::DungeonRightWall, "Right wall", MINE, 1264, 736, 16, 64, false, true},
        {TileTextureId::DungeonBottom, "Bottom edge", MINE, 1120, 784, 32, 16, false, true},
        {TileTextureId::DungeonBottomLeftCorner1, "Bottom left end", MINE, 1088, 768, 16, 16, false, true},
        {TileTextureId::DungeonBottomRightCorner1, "Bottom right end", MINE, 1248, 768, 16, 16, false, true},
        {TileTextureId::DungeonInnerCornerTopRight, "Inner corner (floor up/right)", MINE, 1104, 768, 16, 16, false, true},
        {TileTextureId::DungeonInnerCornerTopLeft, "Inner corner (floor up/left)", MINE, 1232, 768, 16, 16, false, true},
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

ElementInfo wall(FramePieceId id, const std::string& name, const std::string& sheet, int x, int y, int width, int height) {
    ElementInfo info = make(wallElement(id), name, sheet, x, y, width, height);
    info.footprint = {0, 0, info.tilesWidth, info.tilesHeight};
    return info;
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
        decor(DecorId::StreatLamp, "Street lamp", TOWN, 176, 448, 16, 64),
        decor(DecorId::FenceFront, "Fence (front)", TOWN, 16, 449, 16, 32),
        decor(DecorId::FenceBack, "Fence (back)", TOWN, 16, 400, 16, 32),
        decor(DecorId::PinkFlower, "Pink flower", OUTDOORS, 112, 160, 16, 16),
        decor(DecorId::BlueFlower, "Blue flower", OUTDOORS, 112, 192, 16, 16),
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
        decor(DecorId::PottedPlants, "Potted plants", TOWN, 368, 476, 47, 36),
        decor(DecorId::SmallBlueFlower, "Small blue flower", TOWN, 353, 432, 16, 16),

        portal(PortalId::FarmToMine, "Mine entrance", OUTDOORS, 608, 160, 48, 48),
        portal(PortalId::ToNextLevel, "Ladder to next level", "caverns/level_portals.png", 0, 0, 16, 16),
        portal(PortalId::MineToFarm, "Mine exit", "caverns/level_portals.png", 0, 16, 16, 64),

        wall(FramePieceId::StoneCliftTop, "Cliff top", OUTDOORS, 272, 288, 32, 64),
        wall(FramePieceId::StoneCliftTopLeftCorner, "Cliff top left corner", OUTDOORS, 336, 176, 16, 64),
        wall(FramePieceId::StoneCliftTopRightCorner, "Cliff top right corner", OUTDOORS, 384, 176, 16, 64),
        wall(FramePieceId::StoneCliftDropRight, "Cliff drop (right)", OUTDOORS, 464, 400, 64, 64),
        wall(FramePieceId::StoneCliftDropLeft, "Cliff drop (left)", OUTDOORS, 464, 464, 64, 64),
        wall(FramePieceId::StoneCliftLeftSide, "Cliff left side", OUTDOORS, 304, 224, 16, 48),
        wall(FramePieceId::StoneCliftRightSide, "Cliff right side", OUTDOORS, 256, 224, 16, 48),
        wall(FramePieceId::StoneCliftBottom, "Cliff bottom", OUTDOORS, 208, 256, 16, 16),
        wall(FramePieceId::StoneCliftBottomLeftCorner, "Cliff bottom left corner", OUTDOORS, 192, 256, 16, 16),
        wall(FramePieceId::StoneCliftBottomRightCorner, "Cliff bottom right corner", OUTDOORS, 240, 256, 16, 16),

        wall(FramePieceId::DungeonTopWall, "Top wall", MINE, 1168, 640, 64, 64),
        wall(FramePieceId::DungeonTopLeftCorner1, "Top left corner 1", MINE, 1088, 664, 16, 40),
        wall(FramePieceId::DungeonTopLeftCorner2, "Top left corner 2", MINE, 1104, 640, 32, 80),
        wall(FramePieceId::DungeonTopRightCorner1, "Top right corner 1", MINE, 1232, 656, 32, 80),
        wall(FramePieceId::DungeonTopRightCorner2, "Top right corner 2", MINE, 1264, 692, 16, 44),
        wall(FramePieceId::DungeonBottomLeftCorner2, "Bottom left corner 2", MINE, 1104, 768, 16, 32),
        wall(FramePieceId::DungeonBottomRightCorner2, "Bottom right corner 2", MINE, 1232, 768, 16, 32),
        wall(FramePieceId::DungeonTopWallEndLeft, "Top wall end (left)", MINE, 1151, 640, 16, 64),
        wall(FramePieceId::DungeonTopWallEndRight, "Top wall end (right)", MINE, 1137, 640, 16, 64),
        wall(FramePieceId::DungeonLantern, "Lantern", MINE, 1040, 672, 16, 16),
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
ElementId wallElement(FramePieceId id) { return {ElementKind::Wall, static_cast<int>(id)}; }

const ElementInfo& ElementCatalog::infoOf(ElementId element) {
    for (const ElementInfo& info: elements()) {
        if (info.element == element) {
            return info;
        }
    }
    throw std::out_of_range("elemento no en el catalogo");
}

const std::vector<ElementInfo>& ElementCatalog::all() { return elements(); }