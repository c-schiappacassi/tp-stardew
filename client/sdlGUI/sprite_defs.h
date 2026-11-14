#pragma once

#include <cstdint>
#include <initializer_list>
#include "dtos.h"
#include "protocol_types.h"
#include "utils.h"

// TODO: organizar mejor este archivo

namespace Dimentions {
    constexpr uint16_t FARMER_COLS    = 6;
    constexpr uint16_t FARMER_CELL_W  = 16;
    constexpr uint16_t FARMER_CELL_H  = 32;
    constexpr uint16_t FARMER_FEET_Y  = 30;

    constexpr uint16_t COW_COLS       = 4;
    constexpr uint16_t COW_CELL_W     = 32;
    constexpr uint16_t COW_CELL_H     = 32;

    constexpr uint16_t CHICKEN_COLS   = 4;
    constexpr uint16_t CHICKEN_CELL_W = 16;
    constexpr uint16_t CHICKEN_CELL_H = 16;

    constexpr uint16_t ORE_SIZE       = 16;
    constexpr uint16_t CROP_CELL_W    = 16;
    constexpr uint16_t CROP_CELL_H    = 32;

    constexpr uint16_t SKELETON_COLS   = 4;
    constexpr uint16_t SKELETON_CELL_W = 16;
    constexpr uint16_t SKELETON_CELL_H = 32;
    constexpr uint16_t SKELETON_FEET_Y = 30;

    constexpr uint16_t GOLEM_COLS   = 4;
    constexpr uint16_t GOLEM_CELL_W = 16;
    constexpr uint16_t GOLEM_CELL_H = 24;
    constexpr uint16_t GOLEM_FEET_Y = 22;

    constexpr uint16_t BAT_COLS   = 4;
    constexpr uint16_t BAT_CELL_W = 16;
    constexpr uint16_t BAT_CELL_H = 16;
    constexpr uint16_t BAT_CELL_FRENZY_H = 17;
    constexpr uint16_t BAT_FEET_Y = 13;
    constexpr uint16_t BAT_FRENZY_FEET_Y = 15;

    constexpr uint16_t SKULL_COLS    = 2;
    constexpr uint16_t SKULL_CELL_W  = 16;
    constexpr uint16_t SKULL_CELL_H  = 16;
    constexpr uint16_t SKULL_HOVER_Y = 6;

    constexpr uint16_t WALL_FLAME_MAX_H = 19;

    constexpr uint16_t SPIKE_COLS   = 3;
    constexpr uint16_t SPIKE_CELL_W = 16;
    constexpr uint16_t SPIKE_CELL_H = 16;

    constexpr uint16_t FLAME_COLS   = 7;
    constexpr uint16_t FLAME_CELL_W = 8;
    constexpr uint16_t FLAME_CELL_H = 8;
}

namespace SpriteDefs {

    struct TextureDef { SpriteId id; const char* path; };

    inline constexpr TextureDef TEXTURES[] = {
        // ui
        {SpriteId::MenuTiles,    "menus/menutiles.png"},
        {SpriteId::Cursors,      "menus/cursors.png"},
        {SpriteId::Emotes,      "misc/emotes.png"},

        // farmer
        {SpriteId::FarmerBase,   "characters/farmer_base.png"},
        {SpriteId::Hairstyles,   "characters/hairstyles.png"},
        {SpriteId::Pants,        "characters/pants.png"},
        {SpriteId::Shirts,       "characters/shirts.png"},
        {SpriteId::Tools,        "characters/tools.png"},
        {SpriteId::Weapons,      "characters/weapons.png"},

        {SpriteId::Ores,         "caverns/mines.png"},
        {SpriteId::Crops,        "farm/crops/crops.png"},

        //animals
        {SpriteId::BronwCow,     "farm/animals/cow_brown.png"},
        {SpriteId::WhiteCow,     "farm/animals/cow_white.png"},
        {SpriteId::BronwChicken, "farm/animals/chicken_brown.png"},
        {SpriteId::WhiteChicken, "farm/animals/chicken_white.png"},
        {SpriteId::BlueChicken, "farm/animals/chicken_blue.png"},
        {SpriteId::VoidChicken, "farm/animals/chicken_void.png"},

        // enviroment
        {SpriteId::OutdoorsSpring,  "enviroment/outdoors_spring.png"},
        {SpriteId::Trees,           "enviroment/trees.png"},
        {SpriteId::MarketBuildings, "market/world/buildings.png"},
        {SpriteId::FarmBuildings,   "farm/environment/farm_buildings.png"},
        {SpriteId::Cabins,          "farm/environment/cabins.png"},
        {SpriteId::Mushrooms,       "enviroment/mushroom.png"},
        {SpriteId::GrassDecor,      "enviroment/grass.png"},
        {SpriteId::Bushes,          "enviroment/bushes.png"},
        {SpriteId::TownSpring,      "market/world/town_spring.png"},
        {SpriteId::CavernPlants,    "caverns/plants.png"},
        {SpriteId::DungeonWallTiles, "caverns/wall_tiles.png"},
        {SpriteId::Furniture,       "farm/environment/furniture.png"},
        {SpriteId::LevelPortals,    "caverns/level_portals.png"},
        {SpriteId::TilledDirt,      "farm/environment/tilled_dirt.png"},
    

        // monsters
        {SpriteId::StoneGolem, "caverns/monsters/stone_golem.png"},
        {SpriteId::Skeleton, "caverns/monsters/skeleton.png"},
        {SpriteId::FrostBat, "caverns/monsters/frost_bat.png"},
        {SpriteId::FrostBatFrenzy, "caverns/monsters/frost_bat_frenzy.png"},
        {SpriteId::LavaBat, "caverns/monsters/lava_bat.png"},
        {SpriteId::LavaBatFrenzy, "caverns/monsters/lava_bat_frenzy.png"},
        {SpriteId::Skull, "caverns/monsters/skull.png"},

        // traps
        {SpriteId::SpikeTrap, "caverns/monsters/spikes.png"},
        {SpriteId::WallFlameTrap, "caverns/monsters/wallflame.png"},
        {SpriteId::Flames, "caverns/monsters/projectile_fire.png"},
    };

    // Ores
    struct OreDef { ResourceType type; ResourceState state; uint16_t x, y; };
    inline constexpr OreDef ORES[] = {
        {ResourceType::StoneRock,  ResourceState::Intact, 224, 16},
        {ResourceType::StoneRock,  ResourceState::Broken, 240, 16},
        {ResourceType::CoalRock,   ResourceState::Intact,   0, 16},
        {ResourceType::CoalRock,   ResourceState::Broken,  16, 16},
        {ResourceType::LapisRock,  ResourceState::Intact, 224,  0},
        {ResourceType::LapisRock,  ResourceState::Broken, 208,  0},
        {ResourceType::SilverRock, ResourceState::Intact, 192,  0},
        {ResourceType::SilverRock, ResourceState::Broken, 208,  0},
        {ResourceType::GoldRock,   ResourceState::Intact, 128,  0},
        {ResourceType::GoldRock,   ResourceState::Broken, 144,  0},
    };

    // Crops
    struct CropDef { CropType type; CropState state; uint16_t x, y; };
    inline constexpr CropDef CROPS[] = {
        {CropType::Potato,     CropState::JustPlanted, 160,  29},
        {CropType::Potato,     CropState::Sprouting,   176,  29},
        {CropType::Potato,     CropState::Growing,     208,  29},
        {CropType::Potato,     CropState::Ready,       224,  29},

        {CropType::Pumpkin,    CropState::JustPlanted,   0, 285},
        {CropType::Pumpkin,    CropState::Sprouting,    48, 285},
        {CropType::Pumpkin,    CropState::Growing,      80, 285},
        {CropType::Pumpkin,    CropState::Ready,       285, 285},

        {CropType::Strawberry, CropState::JustPlanted, 144, 318},
        {CropType::Strawberry, CropState::Sprouting,   192, 318},
        {CropType::Strawberry, CropState::Growing,     240, 318},
        {CropType::Strawberry, CropState::Ready,       224, 318},

        {CropType::Scallion,   CropState::JustPlanted,  16,  61},
        {CropType::Scallion,   CropState::Sprouting,    48,  61},
        {CropType::Scallion,   CropState::Growing,      64,  61},
        {CropType::Scallion,   CropState::Ready,        80,  61},
    };

    // Map background
    struct SpriteRect { SpriteId sprite; uint16_t x, y, w, h; };

    struct TileDef      { TileTextureId id; SpriteRect rect; };
    struct DecorDef     { DecorId id;       SpriteRect rect; };
    struct BuildingDef  { BuildingId id;    SpriteRect rect; };
    struct FurnitureDef { FurnitureId id; Direction dir; SpriteRect rect; };
    struct PortalDef { PortalId id; SpriteRect rect; };

    inline constexpr TileDef TILES[] = {
        {TileTextureId::Grass,                           {SpriteId::OutdoorsSpring, 0, 112, 16, 16}},
        {TileTextureId::Dirt,                            {SpriteId::OutdoorsSpring, 16, 112, 16, 16}},
        {TileTextureId::StoneCliftTop,                   {SpriteId::OutdoorsSpring, 276, 328, 59, 63}},
        {TileTextureId::StoneCliftSide,                  {SpriteId::OutdoorsSpring, 257, 221, 15, 32}},
        {TileTextureId::StoneCliftTopRightCorner,        {SpriteId::OutdoorsSpring, 384, 176, 16, 64}},
        {TileTextureId::StoneCliftBottom,                {SpriteId::OutdoorsSpring, 208, 256, 16, 32}},
        {TileTextureId::StoneCliftBottomRigthCorner,     {SpriteId::OutdoorsSpring, 240, 256, 16, 16}},
        {TileTextureId::DungeonWall,                     {SpriteId::DungeonWallTiles, 26, 491, 29, 37}},
        {TileTextureId::DungeonWallCorner,               {SpriteId::DungeonWallTiles, 55, 487, 9, 41}},
        {TileTextureId::StonePath,                       {SpriteId::TownSpring, 0, 368, 32, 32}},
        {TileTextureId::ConnctionStonePathGrassCorner,   {SpriteId::TownSpring, 0, 304, 16, 16}},
        {TileTextureId::ConnctionStonePathGrassLeftSide, {SpriteId::TownSpring, 0, 320, 16, 16}},
        {TileTextureId::ConnctionStonePathGrassTop,      {SpriteId::TownSpring, 16, 304, 16, 16}},
        {TileTextureId::ConnctionStonePathGrassBottom,   {SpriteId::TownSpring, 16, 336, 16, 16}},
        {TileTextureId::TilledDirt,   {SpriteId::TilledDirt, 0,0, 16, 16}},
        {TileTextureId::WateredDirt,   {SpriteId::TilledDirt, 64,0, 16, 16}},
    };

    inline constexpr DecorDef DECOR[] = {
        {DecorId::MapleTree,         {SpriteId::Trees, 0, 0, 48, 97}},
        {DecorId::PineTree,          {SpriteId::Trees, 0, 320, 48, 97}},
        {DecorId::MushroomTree,      {SpriteId::Mushrooms, 0, 14, 48, 80}},
        {DecorId::PinkTree,          {SpriteId::Trees, 292, 160, 48, 97}},
        {DecorId::YellowTree,        {SpriteId::Trees, 292, 480, 48, 97}},
        {DecorId::BlueTree,          {SpriteId::Trees, 438, 1120, 48, 97}},
        {DecorId::PinkFlowerGrass,   {SpriteId::GrassDecor, 0, 0, 15, 20}},
        {DecorId::OrangeFlowerGrass, {SpriteId::GrassDecor, 0, 20, 15, 20}},
        {DecorId::OrangeGrass,       {SpriteId::GrassDecor, 0, 40, 15, 20}},
        {DecorId::VelvetGrass,       {SpriteId::GrassDecor, 15, 100, 15, 20}},
        {DecorId::BlueGrass,         {SpriteId::GrassDecor, 15, 160, 15, 20}},
        {DecorId::PlainGrass,        {SpriteId::GrassDecor, 0, 60, 15, 20}},
        {DecorId::RedGrass,          {SpriteId::GrassDecor, 15, 40, 15, 20}},
        {DecorId::SimpleBush,        {SpriteId::Bushes, 0, 9, 32, 36}},
        {DecorId::RedBBerryBush,     {SpriteId::Bushes, 32, 9, 32, 36}},
        {DecorId::PinkBush,          {SpriteId::Bushes, 32, 95, 32, 32}},
        {DecorId::PurpleBerryBush,   {SpriteId::Bushes, 32, 57, 32, 36}},
        {DecorId::BlueBush,          {SpriteId::Bushes, 64, 57, 32, 36}},
        {DecorId::BigRedBush,        {SpriteId::Bushes, 48, 132, 48, 43}},
        {DecorId::StreatLamp,        {SpriteId::TownSpring, 176, 448, 16, 64}},
        {DecorId::FenceFront,        {SpriteId::TownSpring, 16, 449, 16, 32}},
        {DecorId::FenceBack,         {SpriteId::TownSpring, 16, 400, 16, 32}},
        {DecorId::PinkFlower,        {SpriteId::OutdoorsSpring, 112, 160, 16, 16}},
        {DecorId::BlueFlower,        {SpriteId::OutdoorsSpring, 112, 192, 16, 16}},
        {DecorId::WeirdGreenPlant,   {SpriteId::CavernPlants, 0, 0, 16, 24}},
        {DecorId::SmallFern,         {SpriteId::CavernPlants, 16, 0, 16, 24}},
        {DecorId::BigFern,           {SpriteId::CavernPlants, 32, 0, 16, 24}},
        {DecorId::PinkMushrom,       {SpriteId::CavernPlants, 96, 0, 16, 24}},
        {DecorId::YellowMushrom,     {SpriteId::CavernPlants, 128, 0, 16, 24}},
        {DecorId::BabyRedMushrom,    {SpriteId::Mushrooms, 0, 128, 16, 16}},
        {DecorId::BigRedMushroom,    {SpriteId::Mushrooms, 0, 102, 16, 25}},
        {DecorId::DungeonPilar,      {SpriteId::DungeonWallTiles, 0, 384, 16, 48}},
        {DecorId::DungeonTorch,      {SpriteId::DungeonWallTiles, 64, 388, 16, 27}},
        {DecorId::DungeonChain,      {SpriteId::DungeonWallTiles, 64, 416, 16, 32}},
        {DecorId::DungeonWindow,     {SpriteId::DungeonWallTiles, 64, 448, 16, 32}},
        {DecorId::DungeonWallCrack,  {SpriteId::DungeonWallTiles, 64, 480, 16, 32}},
        {DecorId::SpiderWeb,         {SpriteId::GrassDecor, 15, 140, 16, 20}},
        {DecorId::CornerSpiderWeb,   {SpriteId::GrassDecor, 31, 140, 16, 20}},
        {DecorId::PottedPlants,      {SpriteId::TownSpring, 368, 476, 47, 36}},
        {DecorId::SmallBlueFlower,   {SpriteId::TownSpring, 353, 432, 16, 16}},
    };

    inline constexpr FurnitureDef FURNITURE[] = {
        {FurnitureId::Chair,               Direction::Down,   {SpriteId::Furniture,   0,    0, 16, 32}},
        {FurnitureId::Chair,               Direction::Up,     {SpriteId::Furniture,  32,    0, 16, 32}},
        {FurnitureId::Chair,               Direction::Right,  {SpriteId::Furniture,  16,    0, 16, 32}},
        {FurnitureId::Table,               Direction::None,   {SpriteId::Furniture,   0,  569, 32, 39}},
        {FurnitureId::Bed,                 Direction::None,   {SpriteId::Furniture,   0, 1032, 32, 55}},
        {FurnitureId::BedSheet,            Direction::None,   {SpriteId::Furniture,  32, 1032, 32, 55}},
        {FurnitureId::Bonsai,              Direction::None,   {SpriteId::Furniture, 514,  633, 32, 48}},
        {FurnitureId::PottedTree,          Direction::None,   {SpriteId::Furniture, 514,  585, 32, 48}},
        {FurnitureId::BlueArmCouch,        Direction::Down,   {SpriteId::Furniture,   0,  144, 32, 32}},
        {FurnitureId::BlueArmCouch,        Direction::Right,  {SpriteId::Furniture,  32,  144, 32, 32}},
        {FurnitureId::BlueArmCouch,        Direction::Up,     {SpriteId::Furniture,  64,  144, 32, 32}},
        {FurnitureId::Tv,                  Direction::None,   {SpriteId::Furniture, 259,  832, 26, 32}},
        {FurnitureId::BeigeRug,            Direction::None,   {SpriteId::Furniture, 229,  871, 24, 20}},
        {FurnitureId::ClockFirePlace,      Direction::None,   {SpriteId::Furniture, 160,  938, 32, 70}},
        {FurnitureId::BlueRug,             Direction::None,   {SpriteId::Furniture, 149,  867, 37, 28}},
        {FurnitureId::CatFirePlace,        Direction::None,   {SpriteId::Furniture, 128,  925, 32, 51}},
        {FurnitureId::SimpleFirePlace,     Direction::None,   {SpriteId::Furniture,   0,  896, 32, 80}},
        {FurnitureId::BlueJunimoPushee,    Direction::None,   {SpriteId::Furniture,  16,  880, 16, 16}},
        {FurnitureId::GreenJunimoPushee,   Direction::None,   {SpriteId::Furniture,   0,  880, 16, 16}},
        {FurnitureId::YellowJunimoPushee,  Direction::None,   {SpriteId::Furniture,  32,  880, 16, 16}},
        {FurnitureId::PinkJunimoPushee,    Direction::None,   {SpriteId::Furniture,  48,  880, 16, 16}},
        {FurnitureId::SimpleLamp,          Direction::None,   {SpriteId::Furniture,  49,  733, 16, 34}},
        {FurnitureId::BookShelf,           Direction::None,   {SpriteId::Furniture,  80,  640, 32, 48}},
        {FurnitureId::SquareLamp,          Direction::None,   {SpriteId::Furniture,  80,  729, 16, 39}},
        {FurnitureId::RoundTable,          Direction::None,   {SpriteId::Furniture, 224,  570, 32, 38}},
        {FurnitureId::GreenStool,          Direction::None,   {SpriteId::Furniture, 480,   47, 16, 17}},
        {FurnitureId::BrownSofa,           Direction::Down,   {SpriteId::Furniture,   0,  256, 48, 32}},
        {FurnitureId::BrownSofa,           Direction::Right,  {SpriteId::Furniture,  48,  256, 32, 48}},
        {FurnitureId::BrownSofa,           Direction::Up,     {SpriteId::Furniture,  80,  256, 48, 32}},
        {FurnitureId::BrownArmCouch,       Direction::Down,   {SpriteId::Furniture, 384,  144, 32, 32}},
        {FurnitureId::BrownArmCouch,       Direction::Right,  {SpriteId::Furniture, 416,  144, 32, 32}},
        {FurnitureId::BrownArmCouch,       Direction::Up,     {SpriteId::Furniture, 448,  144, 32, 32}},
    };

    inline constexpr BuildingDef BUILDINGS[] = {
        {BuildingId::ComunityCenter, {SpriteId::MarketBuildings, 0, 160, 192, 160}},
        {BuildingId::PierresShop,    {SpriteId::MarketBuildings, 0, 0, 161, 160}},
        {BuildingId::ClintsShop,     {SpriteId::MarketBuildings, 161, 0, 130, 127}},
        {BuildingId::MarniesShop,    {SpriteId::MarketBuildings, 291, 0, 128, 112}},
        {BuildingId::RobinsShop,     {SpriteId::MarketBuildings, 192, 127, 157, 193}},
        {BuildingId::FarmHouse,      {SpriteId::FarmBuildings, 157, 268, 144, 145}},
        {BuildingId::StoneCabin,     {SpriteId::Cabins, 80, 336, 80, 112}},
        {BuildingId::WoodCabin,      {SpriteId::Cabins, 80, 0, 80, 112}},
        {BuildingId::BrickCabin,     {SpriteId::Cabins, 80, 112, 80, 112}},
    };

    inline constexpr  PortalDef PORTALS[] = {
        {PortalId::FarmToMine,  {SpriteId::OutdoorsSpring, 608, 160, 48, 48}},
        {PortalId::ToNextLevel,  {SpriteId::LevelPortals, 0, 0, 16, 16}},
        {PortalId::MineToFarm,  {SpriteId::LevelPortals, 0, 16, 16, 64}},
    };

    // Items
    struct ItemDef { ItemId id; SpriteId sprite; uint16_t x, y, w, h; };
    inline constexpr ItemDef ITEMS[] = {
        {ItemId::Pickaxe,     SpriteId::Tools,   304,  97, 16, 16},
        {ItemId::Sword,       SpriteId::Weapons,  48,  16, 16, 16},
        {ItemId::WateringCan, SpriteId::Tools,    32, 228, 16, 11},
        {ItemId::Shovel,      SpriteId::Tools,   304,  33, 16, 16},
        {ItemId::Coal,        SpriteId::Ores,     16, 108, 16, 16},
        {ItemId::Lapis,       SpriteId::Ores,     64, 108, 16, 16},
        {ItemId::Silver,      SpriteId::Ores,    145,  68, 15, 15},
        {ItemId::Gold,        SpriteId::Ores,     94, 108, 16, 16},
    };

    struct EmoteDef { EmoteId id; SpriteId sprite; uint16_t x, y, w, h; };
    inline constexpr EmoteDef EMOTES[] = {
        {EmoteId::Alert, SpriteId::Emotes, 0, 64, 16, 16},
    };

    // UI
    struct UiDef { UiSpriteId id; SpriteId sheet; uint16_t x, y, w, h; };
    inline constexpr UiDef UI[] = {
        {UiSpriteId::HotbarBar,          SpriteId::MenuTiles,   0, 256, 60, 60},
        {UiSpriteId::HotbarSlot,         SpriteId::MenuTiles,  64, 128, 64, 64},
        {UiSpriteId::HotbarSlotOutline,  SpriteId::MenuTiles, 128, 128, 64, 64},
        {UiSpriteId::HotbarSlotSelected, SpriteId::MenuTiles,   0, 896, 64, 64},

        {UiSpriteId::EnergyBar,          SpriteId::Cursors,   256, 408, 12, 56},
        {UiSpriteId::HealthBar,          SpriteId::Cursors,   268, 408, 12, 56},
    };

    // Clips
    struct FrameDef { int index; int ms; };

    constexpr int MAX_FRAMES = 7; // las llamas (projectile_fire.png) tienen 7 frames; el resto de las anims usa <= 5

    template <class State>
    struct ClipDef {
        State state;
        Direction dir;
        bool loop;
        int count;
        FrameDef frames[MAX_FRAMES];
        std::optional<ItemId> item = std::nullopt;
    };

    template <class State>
    constexpr ClipDef<State> clip(State s, Direction d, bool loop, std::initializer_list<FrameDef> f) {
        ClipDef<State> c{s, d, loop, 0, {}};
        for (const auto& fr : f) c.frames[c.count++] = fr;
        return c;
    }

    template <class State>
    constexpr ClipDef<State> clip(ItemId item, State s, Direction d, bool loop, std::initializer_list<FrameDef> f) {
        ClipDef<State> c{s, d, loop, 0, {}, item};
        for (const auto& fr : f) c.frames[c.count++] = fr;
        return c;
    }

    template <class State>
    constexpr ClipDef<State> clip_run(State s, Direction d, bool loop, int start, int count, int ms) {
        ClipDef<State> c{s, d, loop, 0, {}};
        for (int i = 0; i < count; ++i) c.frames[c.count++] = FrameDef{start + i, ms};
        return c;
    }

    struct RectFrame { uint16_t x, y, w, h; int ms; };

    template <class State>
    struct RectClipDef {
        State state;
        Direction dir;
        bool loop;
        int count;
        RectFrame frames[MAX_FRAMES];
    };

    template <class State>
    constexpr RectClipDef<State> rect_clip(State s, Direction d, bool loop, std::initializer_list<RectFrame> f) {
        RectClipDef<State> c{s, d, loop, 0, {}};
        for (const auto& fr : f) c.frames[c.count++] = fr;
        return c;
    }

    struct ClipSetInfo {
        SpriteId sprite;
        uint16_t cols, cell_w, cell_h;
        int16_t off_x, off_y;
    };

    // Farmer
    inline constexpr ClipSetInfo FARMER_SET{ SpriteId::FarmerBase, Dimentions::FARMER_COLS, Dimentions::FARMER_CELL_W, Dimentions::FARMER_CELL_H,
                                             -Dimentions::FARMER_CELL_W / 2, -Dimentions::FARMER_FEET_Y};
    inline constexpr bool FARMER_MIRROR_LEFT = true;

    inline constexpr ClipDef<PlayerState> FARMER_CLIPS[] = {
        clip(PlayerState::Idle, Direction::Down,  false, {{0, 200}}),
        clip(PlayerState::Idle, Direction::Left,  false, {{6, 200}}),
        clip(PlayerState::Idle, Direction::Right, false, {{6, 200}}),
        clip(PlayerState::Idle, Direction::Up,    false, {{12, 200}}),

        clip(PlayerState::Walking, Direction::Down,  true, {{0, 200}, {1, 200}, {0, 200}, {2, 200}}),
        clip(PlayerState::Walking, Direction::Left,  true, {{6, 200}, {7, 200}, {6, 200}, {8, 200}}),
        clip(PlayerState::Walking, Direction::Right, true, {{6, 200}, {7, 200}, {6, 200}, {8, 200}}),
        clip(PlayerState::Walking, Direction::Up,    true, {{12, 200}, {13, 200}, {12, 200}, {14, 200}}),

        clip_run(PlayerState::UsingTool, Direction::Down,  false, 66, 5, 100),
        clip_run(PlayerState::UsingTool, Direction::Right, false, 48, 5, 100),
        clip_run(PlayerState::UsingTool, Direction::Left,  false, 48, 5, 100),
        clip_run(PlayerState::UsingTool, Direction::Up,    false, 36, 5, 100),

        clip(ItemId::WateringCan, PlayerState::UsingTool, Direction::Down,  false, {{54,150},{55,150},{55,150},{55,150},{54,150}}),
        clip(ItemId::WateringCan, PlayerState::UsingTool, Direction::Right, false, {{58,150},{59,150},{59,150},{59,150},{58,150}}),
        clip(ItemId::WateringCan, PlayerState::UsingTool, Direction::Left,  false, {{58,150},{59,150},{59,150},{59,150},{58,150}}),
        clip(ItemId::WateringCan, PlayerState::UsingTool, Direction::Up,    false, {{62,150},{63,150},{63,150},{63,150},{62,150}}),

        clip(ItemId::Sword, PlayerState::UsingTool, Direction::Down,  false, {{24,100},{25,70},{26,60},{27,60},{29,150}}),
        clip(ItemId::Sword, PlayerState::UsingTool, Direction::Right, false, {{30,100},{31,70},{32,60},{33,60},{35,150}}),
        clip(ItemId::Sword, PlayerState::UsingTool, Direction::Left,  false, {{30,100},{31,70},{32,60},{33,60},{35,150}}),
    };

    struct FarmerOffsetDef { int frame; LayerOffsets off; };

    inline constexpr FarmerOffsetDef FARMER_OFFSETS[] = {
        // frame - hair - shirt - pants
        // Idle / walking down
        {  0, {{ 0,  0}, { 0,  0}, { 0, 0}} },
        {  1, {{ 0,  1}, { 0,  1}, { 0, 0}} },
        {  2, {{ 0,  1}, { 0,  1}, { 0, 0}} },

        // Idle / walking side
        {  6, {{ 0,  -1}, { 0,  0}, { 0, 0}} },
        {  7, {{ 0,  0}, { 0,  1}, { 0, 0}} },
        {  8, {{ 0,  0}, { 0,  1}, { 0, 0}} },

        // Idle / walking up
        { 12, {{ 0,  0}, { 0,  0}, { 0, 0}} },
        { 13, {{ 0,  1}, { 0,  0}, { 0, 0}} },
        { 14, {{ 0,  1}, { 0,  0}, { 0, 0}} },

        // Using tool up
        { 36, {{  0,  0}, {  0,  0}, {0, 0}} },
        { 37, {{  0,  0}, {  0,  0}, {0, 0}} },
        { 38, {{  0, -1}, {  0, -1}, {0, 0}} },
        { 39, {{  0, -1}, {  0, -1}, {0, 0}} },
        { 40, {{  0,  0}, {  0,  0}, {0, 0}} },

        // Using tool side
        { 48, {{ -1,  -1}, { -1,  0}, {0, 0}} },
        { 49, {{  0, -2}, {  0, -1}, {0, 0}} },
        { 50, {{  0, -3}, {  0, -2}, {0, 0}} },
        { 51, {{  0, -4}, {  0, -3}, {0, 0}} },
        { 52, {{  0, -3}, {  0, -2}, {0, 0}} },

        // Using tool down
        { 66, {{  0,  1}, {  0,  1}, {0, 0}} },
        { 67, {{  0,  0}, {  0,  0}, {0, 0}} },
        { 68, {{  0, -1}, {  0, -1}, {0, 0}} },
        { 69, {{  0, -2}, {  0, -2}, {0, 0}} },
        { 70, {{  0,  0}, {  0,  0}, {0, 0}} },

        // Watering can down
        { 54, {{0, 4}, {0, 4}, {0, 0}} },
        { 55, {{0, 3}, {0, 3}, {0, 0}} },
        // Watering can side
        { 58, {{ 0, 1}, { 0, 2}, {0, 0}} },
        { 59, {{-1, 0}, {-1, 1}, {0, 0}} },
        // Watering can up
        { 62, {{0, 3}, {0, 3}, {0, 0}} },
        { 63, {{0, 1}, {0, 1}, {0, 0}} },

        // Sword down
        { 24, {{0, -1}, {0, -1}, {0, 0}} },
        { 25, {{0, -1}, {0, -1}, {0, 0}} },
        { 26, {{0,  1}, {0,  1}, {0, 0}} },
        { 27, {{0,  1}, {0,  1}, {0, 0}} },
        { 29, {{0,  3}, {0,  3}, {0, 0}} },
        // Sword side
        { 30, {{0, -2}, {0, -1}, {0, 0}} },
        { 31, {{0, -2}, {0, -1}, {0, 0}} },
        { 32, {{0, -1}, {0,  0}, {0, 0}} },
        { 33, {{0,  0}, {0,  1}, {0, 0}} },
        { 35, {{0, -1}, {0,  0}, {0, 0}} },
    };

    // Tools
    struct ToolRect { uint16_t x, y, w, h; };
    struct ToolStep { int sprite; Offset off; double angle; };

    struct ToolAnimDef {
        ItemId item;
        Direction dir;
        ToolRect sprites[2];
        int pivot_x, pivot_y;
        ToolStep steps[5];
        SpriteId sheet = SpriteId::Tools;
    };

    inline const ToolAnimDef TOOL_ANIMS[] = {
        { ItemId::Pickaxe, Direction::Down,
        {{224, 89, 16, 23}, {240, 89, 16, 23}}, 0, 0,
        {{0, {0, -6}, 0}, {0, {0, -8}, 0}, {0, {0, -7}, 0}, {1, {0, 8}, 0}, {1, {0, 8}, 0}} },

        { ItemId::Pickaxe, Direction::Up,
        {{272, 89, 16, 23}, {288, 89, 16, 23}}, 0, 0,
        {{0, {0, -10}, 0}, {0, {0, -12}, 0}, {0, {0, -12}, 0}, {1, {0, -6}, 0}, {1, {0, -4}, 0}} },

        { ItemId::Pickaxe, Direction::Right,
        {{256, 93, 16, 19}, {0, 0, 0, 0}}, 2, 17,
        {{0, {3, -5},  -110}, {0, {6, -7},  -60}, {0, {10, -5}, -10}, {0, {10, -4},   40},  {0, {10, -4},   80}} },

        { ItemId::WateringCan, Direction::Down,
        {{0, 219, 16, 20}, {16, 219, 16, 20}}, 0, 0,
        {{0, {1, 9}, 0}, {1, {1, 8}, 0}, {1, {1, 8}, 0}, {1, {1, 8}, 0}, {0, {1, 9}, 0}} },

        { ItemId::WateringCan, Direction::Right,
        {{32, 219, 16, 20}, {48, 219, 16, 20}}, 0, 0,
        {{0, {5, 8}, 0}, {1, {4, 5}, 0}, {1, {4, 5}, 0}, {1, {4, 5}, 0}, {0, {5, 8}, 0}} },

        { ItemId::WateringCan, Direction::Up,
        {{64, 219, 16, 20}, {64, 219, 16, 20}}, 0, 0,
        {{0, {0, 4}, 0}, {0, {0, 3}, 0}, {0, {0, 3}, 0}, {0, {0, 3}, 0}, {0, {0, 4}, 0}} },

        { ItemId::Sword, Direction::Down,
        {{48, 16, 16, 16}, {0, 0, 0, 0}}, 3, 13,
        {{0, {7, -3}, -30}, {0, {7, -1}, 0}, {0, {7, 2}, 50}, {0, {7, 5}, 95}, {0, {6, 8}, 135}},
        SpriteId::Weapons },

        { ItemId::Sword, Direction::Right,
        {{48, 16, 16, 16}, {0, 0, 0, 0}}, 3, 13,
        {{0, {0, -5}, -70}, {0, {9, -5}, -15}, {0, {10, -2}, 25}, {0, {11, 3}, 65}, {0, {12, 5}, 110}},
        SpriteId::Weapons },

        { ItemId::Sword, Direction::Up,
        {{48, 16, 16, 16}, {0, 0, 0, 0}}, 3, 13,
        {{0, {9, 1}, 70}, {0, {9, 0}, 30}, {0, {9, -1}, -10}, {0, {9, -1}, -45}, {0, {9, 0}, -80}},
        SpriteId::Weapons },

        { ItemId::Shovel, Direction::Down,
        {{224, 26, 16, 22}, {240, 26, 16, 22}}, 0, 0,
        {{0, {0, -5}, 0}, {0, {0, -7}, 0}, {0, {0, -6}, 0}, {1, {0, 9}, 0}, {1, {0, 9}, 0}} },

        { ItemId::Shovel, Direction::Up,
        {{272, 26, 16, 22}, {288, 26, 16, 22}}, 0, 0,
        {{0, {0, -9}, 0}, {0, {0, -11}, 0}, {0, {0, -11}, 0}, {1, {0, -9}, 0}, {1, {0, -7}, 0}} },

        { ItemId::Shovel, Direction::Right,
        {{256, 26, 16, 22}, {0, 0, 0, 0}}, 9, 20,
        {{0, {-6, -9}, -30}, {0, {-2, -10}, 0}, {0, {4, -7}, 40}, {0, {5, -5}, 80}, {0, {5, -4}, 100}} },
    };

    constexpr LayerOffsets farmer_offsets(int frame) {
        for (const auto& o : FARMER_OFFSETS)
            if (o.frame == frame) return o.off;
        return {};
    }

    // Cow
    inline constexpr ClipSetInfo COW_SET{SpriteId::BronwCow, Dimentions::COW_COLS, Dimentions::COW_CELL_W, Dimentions::COW_CELL_H, -Dimentions::COW_CELL_W / 2, -Dimentions::COW_CELL_H};
    inline constexpr int COW_IDLE_MS = 400;
    inline constexpr int COW_WALK_MS = 150;

    inline constexpr ClipDef<AnimalState> COW_CLIPS[] = {
        clip_run(AnimalState::Idle, Direction::Down,  true, 0, 1, COW_IDLE_MS),
        clip_run(AnimalState::Idle, Direction::Right, true, 4, 1, COW_IDLE_MS),
        clip_run(AnimalState::Idle, Direction::Up,    true, 8, 1, COW_IDLE_MS),
        clip_run(AnimalState::Idle, Direction::Left,  true, 4, 1, COW_IDLE_MS),

        clip_run(AnimalState::Walking, Direction::Down,  true, 0, 4, COW_WALK_MS),
        clip_run(AnimalState::Walking, Direction::Right, true, 4, 4, COW_WALK_MS),
        clip_run(AnimalState::Walking, Direction::Up,    true, 8, 4, COW_WALK_MS),
        clip_run(AnimalState::Walking, Direction::Left,  true, 4, 4, COW_WALK_MS),
    };

    // Chicken
    inline constexpr ClipSetInfo CHICKEN_SET{ SpriteId::BronwChicken, Dimentions::CHICKEN_COLS, Dimentions::CHICKEN_CELL_W, Dimentions::CHICKEN_CELL_H,
                                              -Dimentions::CHICKEN_CELL_W / 2, -Dimentions::CHICKEN_CELL_H};

    inline constexpr int CHICKEN_IDLE_MS = 400;
    inline constexpr int CHICKEN_WALK_MS = 120;
    inline constexpr int CHICKEN_DOWN_ROW  = 0 * Dimentions::CHICKEN_COLS;
    inline constexpr int CHICKEN_RIGHT_ROW = 1 * Dimentions::CHICKEN_COLS;
    inline constexpr int CHICKEN_UP_ROW    = 2 * Dimentions::CHICKEN_COLS;

    inline constexpr ClipDef<AnimalState> CHICKEN_CLIPS[] = {
        clip_run(AnimalState::Idle, Direction::Down,  true, CHICKEN_DOWN_ROW,  1, CHICKEN_IDLE_MS),
        clip_run(AnimalState::Idle, Direction::Right, true, CHICKEN_RIGHT_ROW, 1, CHICKEN_IDLE_MS),
        clip_run(AnimalState::Idle, Direction::Up,    true, CHICKEN_UP_ROW,    1, CHICKEN_IDLE_MS),
        clip_run(AnimalState::Idle, Direction::Left,  true, CHICKEN_RIGHT_ROW, 1, CHICKEN_IDLE_MS),

        clip_run(AnimalState::Walking, Direction::Down,  true, CHICKEN_DOWN_ROW,  4, CHICKEN_WALK_MS),
        clip_run(AnimalState::Walking, Direction::Right, true, CHICKEN_RIGHT_ROW, 4, CHICKEN_WALK_MS),
        clip_run(AnimalState::Walking, Direction::Up,    true, CHICKEN_UP_ROW,    4, CHICKEN_WALK_MS),
        clip_run(AnimalState::Walking, Direction::Left,  true, CHICKEN_RIGHT_ROW, 4, CHICKEN_WALK_MS),
    };


    inline constexpr ClipSetInfo SKELETON_SET{
    SpriteId::Skeleton, Dimentions::SKELETON_COLS,
    Dimentions::SKELETON_CELL_W, Dimentions::SKELETON_CELL_H,
    -Dimentions::SKELETON_CELL_W / 2, -Dimentions::SKELETON_FEET_Y};

    inline constexpr ClipSetInfo SKELETON_BONES_SET{
        SpriteId::Skeleton, Dimentions::SKELETON_COLS,
        Dimentions::SKELETON_CELL_W, 16,
        -Dimentions::SKELETON_CELL_W / 2, -16};

    inline constexpr int SKELETON_IDLE_MS   = 400;
    inline constexpr int SKELETON_WALK_MS   = 160;
    inline constexpr int SKELETON_ATTACK_MS = 120;

    inline constexpr ClipDef<EnemyState> SKELETON_CLIPS[] = {
        clip_run(EnemyState::Idle, Direction::Down,  true, 0,  1, SKELETON_IDLE_MS),
        clip_run(EnemyState::Idle, Direction::Right, true, 4,  1, SKELETON_IDLE_MS),
        clip_run(EnemyState::Idle, Direction::Up,    true, 8,  1, SKELETON_IDLE_MS),
        clip_run(EnemyState::Idle, Direction::Left,  true, 12, 1, SKELETON_IDLE_MS),

        clip_run(EnemyState::Moving, Direction::Down,  true, 0,  4, SKELETON_WALK_MS),
        clip_run(EnemyState::Moving, Direction::Right, true, 4,  4, SKELETON_WALK_MS),
        clip_run(EnemyState::Moving, Direction::Up,    true, 8,  4, SKELETON_WALK_MS),
        clip_run(EnemyState::Moving, Direction::Left,  true, 12, 4, SKELETON_WALK_MS),

        clip_run(EnemyState::Attacking, Direction::Down,  true, 20, 4, SKELETON_ATTACK_MS),
        clip_run(EnemyState::Attacking, Direction::Right, true, 4,  4, 100),
        clip_run(EnemyState::Attacking, Direction::Up,    true, 8,  4, 100),
        clip_run(EnemyState::Attacking, Direction::Left,  true, 12, 4, 100),
    };

    inline constexpr ClipDef<EnemyState> SKELETON_DEAD_CLIPS[] = {
        clip_run(EnemyState::Dead, Direction::Down,  true, 32, 1, 400),
        clip_run(EnemyState::Dead, Direction::Right, true, 32, 1, 400),
        clip_run(EnemyState::Dead, Direction::Up,    true, 32, 1, 400),
        clip_run(EnemyState::Dead, Direction::Left,  true, 32, 1, 400),
    };

    inline constexpr ClipSetInfo GOLEM_SET{
    SpriteId::StoneGolem, Dimentions::GOLEM_COLS,
    Dimentions::GOLEM_CELL_W, Dimentions::GOLEM_CELL_H,
    -Dimentions::GOLEM_CELL_W / 2, -Dimentions::GOLEM_FEET_Y};

    inline constexpr ClipSetInfo GOLEM_ROCKS_SET{
        SpriteId::StoneGolem, Dimentions::GOLEM_COLS,
        Dimentions::GOLEM_CELL_W, 16,
        -Dimentions::GOLEM_CELL_W / 2, -16};

    inline constexpr int GOLEM_IDLE_MS   = 400;
    inline constexpr int GOLEM_WALK_MS   = 200;
    inline constexpr int GOLEM_ATTACK_MS = 130;

    inline constexpr ClipDef<EnemyState> GOLEM_CLIPS[] = {
        clip_run(EnemyState::Idle, Direction::Down,  true, 0,  1, GOLEM_IDLE_MS),
        clip_run(EnemyState::Idle, Direction::Right, true, 4,  1, GOLEM_IDLE_MS),
        clip_run(EnemyState::Idle, Direction::Up,    true, 8,  1, GOLEM_IDLE_MS),
        clip_run(EnemyState::Idle, Direction::Left,  true, 12, 1, GOLEM_IDLE_MS),

        clip_run(EnemyState::Moving, Direction::Down,  true, 0,  4, GOLEM_WALK_MS),
        clip_run(EnemyState::Moving, Direction::Right, true, 4,  4, GOLEM_WALK_MS),
        clip_run(EnemyState::Moving, Direction::Up,    true, 8,  4, GOLEM_WALK_MS),
        clip_run(EnemyState::Moving, Direction::Left,  true, 12, 4, GOLEM_WALK_MS),

        clip_run(EnemyState::Attacking, Direction::Down,  true, 0,  4, GOLEM_ATTACK_MS),
        clip_run(EnemyState::Attacking, Direction::Right, true, 4,  4, GOLEM_ATTACK_MS),
        clip_run(EnemyState::Attacking, Direction::Up,    true, 8,  4, GOLEM_ATTACK_MS),
        clip_run(EnemyState::Attacking, Direction::Left,  true, 12, 4, GOLEM_ATTACK_MS),
    };

    inline constexpr ClipDef<EnemyState> GOLEM_DEAD_CLIPS[] = {
        clip_run(EnemyState::Dead, Direction::Down,  true, 36, 1, 400),
        clip_run(EnemyState::Dead, Direction::Right, true, 36, 1, 400),
        clip_run(EnemyState::Dead, Direction::Up,    true, 36, 1, 400),
        clip_run(EnemyState::Dead, Direction::Left,  true, 36, 1, 400),
    };

    inline constexpr ClipSetInfo FROST_BAT_SET{
    SpriteId::FrostBat, Dimentions::BAT_COLS, Dimentions::BAT_CELL_W, Dimentions::BAT_CELL_H,
    -Dimentions::BAT_CELL_W / 2, -Dimentions::BAT_FEET_Y};
    inline constexpr ClipSetInfo LAVA_BAT_SET{
        SpriteId::LavaBat, Dimentions::BAT_COLS, Dimentions::BAT_CELL_W, Dimentions::BAT_CELL_H,
        -Dimentions::BAT_CELL_W / 2, -Dimentions::BAT_FEET_Y};

    inline constexpr ClipSetInfo FROST_BAT_FRENZY_SET{
        SpriteId::FrostBatFrenzy, Dimentions::BAT_COLS, Dimentions::BAT_CELL_W, Dimentions::BAT_CELL_FRENZY_H,
        -Dimentions::BAT_CELL_W / 2, -Dimentions::BAT_FRENZY_FEET_Y};
    inline constexpr ClipSetInfo LAVA_BAT_FRENZY_SET{
        SpriteId::LavaBatFrenzy, Dimentions::BAT_COLS, Dimentions::BAT_CELL_W, Dimentions::BAT_CELL_FRENZY_H,
        -Dimentions::BAT_CELL_W / 2, -Dimentions::BAT_FRENZY_FEET_Y};

    inline constexpr ClipSetInfo FROST_BAT_DEBRIS_SET{
        SpriteId::FrostBat, Dimentions::BAT_COLS, Dimentions::BAT_CELL_W, 16,
        -Dimentions::BAT_CELL_W / 2, -16};
    inline constexpr ClipSetInfo LAVA_BAT_DEBRIS_SET{
        SpriteId::LavaBat, Dimentions::BAT_COLS, Dimentions::BAT_CELL_W, 16,
        -Dimentions::BAT_CELL_W / 2, -16};

    inline constexpr int BAT_FLAP_MS   = 130;
    inline constexpr int BAT_ATTACK_MS = 90;
    inline constexpr int BAT_DEBRIS_FRAME = 1 * Dimentions::BAT_COLS;


    inline constexpr ClipDef<EnemyState> BAT_CLIPS[] = {
        clip_run(EnemyState::Idle, Direction::Down,  true, 0, 4, BAT_FLAP_MS),
        clip_run(EnemyState::Idle, Direction::Right, true, 0, 4, BAT_FLAP_MS),
        clip_run(EnemyState::Idle, Direction::Left,  true, 0, 4, BAT_FLAP_MS),
        clip_run(EnemyState::Idle, Direction::Up,    true, 0, 4, BAT_FLAP_MS),

        clip_run(EnemyState::Moving, Direction::Down,  true, 0, 4, BAT_FLAP_MS),
        clip_run(EnemyState::Moving, Direction::Right, true, 0, 4, BAT_FLAP_MS),
        clip_run(EnemyState::Moving, Direction::Left,  true, 0, 4, BAT_FLAP_MS),
        clip_run(EnemyState::Moving, Direction::Up,    true, 0, 4, BAT_FLAP_MS),

        clip_run(EnemyState::Attacking, Direction::Down,  true, 0, 4, BAT_ATTACK_MS),
        clip_run(EnemyState::Attacking, Direction::Right, true, 0, 4, BAT_ATTACK_MS),
        clip_run(EnemyState::Attacking, Direction::Left,  true, 0, 4, BAT_ATTACK_MS),
        clip_run(EnemyState::Attacking, Direction::Up,    true, 0, 4, BAT_ATTACK_MS),
    };

    inline constexpr ClipDef<EnemyState> BAT_DEAD_CLIPS[] = {
        clip_run(EnemyState::Dead, Direction::Down,  true, BAT_DEBRIS_FRAME, 1, 400),
        clip_run(EnemyState::Dead, Direction::Right, true, BAT_DEBRIS_FRAME, 1, 400),
        clip_run(EnemyState::Dead, Direction::Up,    true, BAT_DEBRIS_FRAME, 1, 400),
        clip_run(EnemyState::Dead, Direction::Left,  true, BAT_DEBRIS_FRAME, 1, 400),
    };

    inline constexpr ClipSetInfo SKULL_SET{
        SpriteId::Skull, Dimentions::SKULL_COLS,
        Dimentions::SKULL_CELL_W, Dimentions::SKULL_CELL_H,
        -Dimentions::SKULL_CELL_W / 2, -(Dimentions::SKULL_CELL_H + Dimentions::SKULL_HOVER_Y)};

    inline constexpr int SKULL_IDLE_MS   = 400;
    inline constexpr int SKULL_MOVE_MS   = 200;
    inline constexpr int SKULL_ATTACK_MS = 120;

    inline constexpr int SKULL_UP    = 0 * Dimentions::SKULL_COLS;
    inline constexpr int SKULL_RIGHT = 1 * Dimentions::SKULL_COLS;
    inline constexpr int SKULL_DOWN  = 2 * Dimentions::SKULL_COLS;
    inline constexpr int SKULL_LEFT  = 3 * Dimentions::SKULL_COLS;

    inline constexpr ClipDef<EnemyState> SKULL_CLIPS[] = {
        clip_run(EnemyState::Idle, Direction::Up,    true, SKULL_UP,    1, SKULL_IDLE_MS),
        clip_run(EnemyState::Idle, Direction::Right, true, SKULL_RIGHT, 1, SKULL_IDLE_MS),
        clip_run(EnemyState::Idle, Direction::Down,  true, SKULL_DOWN,  1, SKULL_IDLE_MS),
        clip_run(EnemyState::Idle, Direction::Left,  true, SKULL_LEFT,  1, SKULL_IDLE_MS),

        clip_run(EnemyState::Moving, Direction::Up,    true, SKULL_UP,    1, SKULL_MOVE_MS),
        clip_run(EnemyState::Moving, Direction::Right, true, SKULL_RIGHT, 1, SKULL_MOVE_MS),
        clip_run(EnemyState::Moving, Direction::Down,  true, SKULL_DOWN,  1, SKULL_MOVE_MS),
        clip_run(EnemyState::Moving, Direction::Left,  true, SKULL_LEFT,  1, SKULL_MOVE_MS),

        clip_run(EnemyState::Attacking, Direction::Up,    true, SKULL_UP    + 1, 1, SKULL_ATTACK_MS),
        clip_run(EnemyState::Attacking, Direction::Right, true, SKULL_RIGHT + 1, 1, SKULL_ATTACK_MS),
        clip_run(EnemyState::Attacking, Direction::Down,  true, SKULL_DOWN  + 1, 1, SKULL_ATTACK_MS),
        clip_run(EnemyState::Attacking, Direction::Left,  true, SKULL_LEFT  + 1, 1, SKULL_ATTACK_MS),
    };

    inline constexpr ClipSetInfo SPIKE_SET{
        SpriteId::SpikeTrap, Dimentions::SPIKE_COLS,
        Dimentions::SPIKE_CELL_W, Dimentions::SPIKE_CELL_H,
        -Dimentions::SPIKE_CELL_W / 2, -Dimentions::SPIKE_CELL_H};

    inline constexpr int SPIKE_STATE_MS = 400;

    inline constexpr ClipDef<TrapState> SPIKE_CLIPS[] = {
        clip_run(TrapState::Inactive, Direction::None, true, 0, 1, SPIKE_STATE_MS),
        clip_run(TrapState::Warning,  Direction::None, true, 1, 1, SPIKE_STATE_MS),
        clip_run(TrapState::Active,   Direction::None, true, 2, 1, SPIKE_STATE_MS),
    };

    inline constexpr int WALL_FLAME_STATE_MS   = 400;
    inline constexpr int WALL_FLAME_WARNING_MS = 150;

    inline constexpr RectClipDef<TrapState> WALL_FLAME_CLIPS[] = {
        rect_clip(TrapState::Inactive, Direction::Down, true,  {{ 0, 22, 11, 15, WALL_FLAME_STATE_MS}}),
        rect_clip(TrapState::Warning,  Direction::Down, false, {{11, 22, 11, 15, WALL_FLAME_WARNING_MS},
                                                                {22, 22, 11, 15, WALL_FLAME_WARNING_MS},
                                                                {33, 22, 11, 15, WALL_FLAME_WARNING_MS}}),
        rect_clip(TrapState::Active,   Direction::Down, true,  {{44, 22, 11, 15, WALL_FLAME_STATE_MS}}),

        rect_clip(TrapState::Inactive, Direction::Up, true,  {{ 0, 37, 11, 19, WALL_FLAME_STATE_MS}}),
        rect_clip(TrapState::Warning,  Direction::Up, false, {{11, 37, 11, 19, WALL_FLAME_WARNING_MS},
                                                              {22, 37, 11, 19, WALL_FLAME_WARNING_MS},
                                                              {33, 37, 11, 19, WALL_FLAME_WARNING_MS}}),
        rect_clip(TrapState::Active,   Direction::Up, true,  {{44, 37, 11, 19, WALL_FLAME_STATE_MS}}),

        rect_clip(TrapState::Inactive, Direction::Right, true,  {{ 0,  0, 15, 11, WALL_FLAME_STATE_MS}}),
        rect_clip(TrapState::Warning,  Direction::Right, false, {{19,  0, 15, 11, WALL_FLAME_WARNING_MS},
                                                                 {39,  0, 15, 11, WALL_FLAME_WARNING_MS},
                                                                 { 0, 11, 15, 11, WALL_FLAME_WARNING_MS}}),
        rect_clip(TrapState::Active,   Direction::Right, true,  {{19, 11, 15, 11, WALL_FLAME_STATE_MS}}),
    };

    inline constexpr ClipSetInfo FLAME_SET{
        SpriteId::Flames, Dimentions::FLAME_COLS,
        Dimentions::FLAME_CELL_W, Dimentions::FLAME_CELL_H,
        -Dimentions::FLAME_CELL_W / 2, -Dimentions::FLAME_CELL_H / 2};

    inline constexpr int FLAME_FRAME_MS = 80;
    inline constexpr int FLAME_FRAMES   = 7;

    inline constexpr ClipDef<FlameType> FLAME_CLIPS[] = {
        clip_run(FlameType::Green,  Direction::None, true, 0 * Dimentions::FLAME_COLS, FLAME_FRAMES, FLAME_FRAME_MS),
        clip_run(FlameType::Blue,   Direction::None, true, 1 * Dimentions::FLAME_COLS, FLAME_FRAMES, FLAME_FRAME_MS),
        clip_run(FlameType::Common, Direction::None, true, 2 * Dimentions::FLAME_COLS, FLAME_FRAMES, FLAME_FRAME_MS),
    };
}