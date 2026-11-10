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
}

namespace SpriteDefs {

    struct TextureDef { SpriteId id; const char* path; };

    inline constexpr TextureDef TEXTURES[] = {
        {SpriteId::MenuTiles,    "menus/menutiles.png"},
        {SpriteId::FarmerBase,   "characters/farmer_base.png"},
        {SpriteId::Hairstyles,   "characters/hairstyles.png"},
        {SpriteId::Pants,        "characters/pants.png"},
        {SpriteId::Shirts,       "characters/shirts.png"},
        {SpriteId::Tools,        "characters/tools.png"},
        {SpriteId::Weapons,      "characters/weapons.png"},
        {SpriteId::Ores,         "caverns/mines.png"},
        {SpriteId::Crops,        "farm/crops/crops.png"},
        {SpriteId::BronwCow,     "farm/animals/cow_brown.png"},
        {SpriteId::BronwChicken, "farm/animals/chicken_brown.png"},
        {SpriteId::OutdoorsSpring,  "enviroment/outdoors_spring.png"},
        {SpriteId::Trees,           "enviroment/trees.png"},
        {SpriteId::MarketBuildings, "market/world/buildings.png"},
        {SpriteId::FarmBuildings,   "farm/environment/farm_buildings.png"},
        {SpriteId::Cabins,          "farm/environment/cabins.png"},
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

    inline constexpr TileDef TILES[] = {
        {TileTextureId::Grass, {SpriteId::OutdoorsSpring,  0, 112, 16, 16}},
        {TileTextureId::Dirt,  {SpriteId::OutdoorsSpring, 16, 112, 16, 16}},
    };

    inline constexpr DecorDef DECOR[] = {
        {DecorId::MapleTree, {SpriteId::Trees, 0,   0, 48, 97}},
        {DecorId::PineTree,  {SpriteId::Trees, 0, 320, 48, 97}},
    };

    inline constexpr BuildingDef BUILDINGS[] = {
        {BuildingId::ComunityCenter, {SpriteId::MarketBuildings,   0, 160, 192, 160}},
        {BuildingId::PierresShop,    {SpriteId::MarketBuildings,   0,   0, 161, 160}},
        {BuildingId::ClintsShop,     {SpriteId::MarketBuildings, 161,   0, 130, 127}},
        {BuildingId::MarniesShop,    {SpriteId::MarketBuildings, 291,   0, 128, 112}},
        {BuildingId::RobinsShop,     {SpriteId::MarketBuildings, 192, 127, 157, 193}},
        {BuildingId::FarmHouse,      {SpriteId::FarmBuildings,   157, 268, 144, 145}},
        {BuildingId::StoneCabin,     {SpriteId::Cabins,           80, 336,  80, 112}},
        {BuildingId::WoodCabin,      {SpriteId::Cabins,           80,   0,  80, 112}},
        {BuildingId::BrickCabin,     {SpriteId::Cabins,           80, 112,  80, 112}},
    };

    inline constexpr FurnitureDef FURNITURE[] = {
        {FurnitureId::Chair, Direction::Down,  {SpriteId::OutdoorsSpring,  0,   0, 16, 32}},
        {FurnitureId::Chair, Direction::Up,    {SpriteId::OutdoorsSpring, 32,   0, 16, 32}},
        {FurnitureId::Chair, Direction::Right, {SpriteId::OutdoorsSpring, 16,   0, 16, 32}},
        {FurnitureId::Chair, Direction::Left,  {SpriteId::OutdoorsSpring, 16,   0, 16, 32}},
        {FurnitureId::Chair, Direction::None,  {SpriteId::OutdoorsSpring,  0,   0, 16, 32}},
        {FurnitureId::Table, Direction::None,  {SpriteId::OutdoorsSpring,  0, 569, 32, 39}},
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

    // UI
    struct UiDef { UiSpriteId id; uint16_t x, y, w, h; };
    inline constexpr UiDef UI[] = {
        {UiSpriteId::HotbarBar,          0, 256, 60, 60},
        {UiSpriteId::HotbarSlot,        64, 128, 64, 64},
        {UiSpriteId::HotbarSlotOutline, 128, 128, 64, 64},
        {UiSpriteId::HotbarSlotSelected,  0, 896, 64, 64},
    };

    // Clips
    struct FrameDef { int index; int ms; };

    constexpr int MAX_FRAMES = 5; // TODO: creo que este es el max en la anim de todo, pero chequear enemigos

    template <class State>
    struct ClipDef {
        State state;
        Direction dir;
        bool loop;
        int count;
        FrameDef frames[MAX_FRAMES];
    };

    template <class State>
    constexpr ClipDef<State> clip(State s, Direction d, bool loop, std::initializer_list<FrameDef> f) {
        ClipDef<State> c{s, d, loop, 0, {}};
        for (const auto& fr : f) c.frames[c.count++] = fr;
        return c;
    }

    template <class State>
    constexpr ClipDef<State> clip_run(State s, Direction d, bool loop, int start, int count, int ms) {
        ClipDef<State> c{s, d, loop, 0, {}};
        for (int i = 0; i < count; ++i) c.frames[c.count++] = FrameDef{start + i, ms};
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
    };

    // TODO: agregar las otras herramientas
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
    };

    constexpr LayerOffsets farmer_offsets(int frame) {
        for (const auto& o : FARMER_OFFSETS)
            if (o.frame == frame) return o.off;
        return {};
    }

    // TODO: cambiar sprite dependiendo del color de vaca y pollo
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

}