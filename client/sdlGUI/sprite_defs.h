#pragma once

#include <cstdint>
#include <initializer_list>
#include "dtos.h"
#include "protocol_types.h"
#include "utils.h"

namespace Dimentions {
    constexpr uint16_t FARMER_COLS    = 6;
    constexpr uint16_t FARMER_CELL_W  = 16;
    constexpr uint16_t FARMER_CELL_H  = 32;

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
    };

    // Ores
    struct OreDef { ResourceType type; ResourceState state; uint16_t row, col; };
    inline constexpr OreDef ORES[] = {
        {ResourceType::StoneRock,  ResourceState::Intact, 1, 14},
        {ResourceType::StoneRock,  ResourceState::Broken, 1, 15},
        {ResourceType::CoalRock,   ResourceState::Intact, 1, 0},
        {ResourceType::CoalRock,   ResourceState::Broken, 1, 1},
        {ResourceType::LapisRock,  ResourceState::Intact, 0, 14},
        {ResourceType::LapisRock,  ResourceState::Broken, 0, 13},
        {ResourceType::SilverRock, ResourceState::Intact, 0, 12},
        {ResourceType::SilverRock, ResourceState::Broken, 0, 13},
        {ResourceType::GoldRock,   ResourceState::Intact, 0, 8},
        {ResourceType::GoldRock,   ResourceState::Broken, 0, 9},
    };

    // Crops
    struct CropDef { CropType type; CropState state; uint16_t row, col; };
    inline constexpr CropDef CROPS[] = {
        {CropType::Potato,     CropState::JustPlanted, 29, 160},
        {CropType::Potato,     CropState::Sprouting,   29, 176},
        {CropType::Potato,     CropState::Growing,     29, 208},
        {CropType::Potato,     CropState::Ready,       29, 224},

        {CropType::Pumpkin,    CropState::JustPlanted, 285, 0},
        {CropType::Pumpkin,    CropState::Sprouting,   285, 48},
        {CropType::Pumpkin,    CropState::Growing,     285, 80},
        {CropType::Pumpkin,    CropState::Ready,       285, 285},

        {CropType::Strawberry, CropState::JustPlanted, 318, 144},
        {CropType::Strawberry, CropState::Sprouting,   318, 192},
        {CropType::Strawberry, CropState::Growing,     318, 240},
        {CropType::Strawberry, CropState::Ready,       318, 224},

        {CropType::Scallion,   CropState::JustPlanted, 61, 16},
        {CropType::Scallion,   CropState::Sprouting,   61, 48},
        {CropType::Scallion,   CropState::Growing,     61, 64},
        {CropType::Scallion,   CropState::Ready,       61, 80},
    };

    // Items
    struct ItemDef { ItemId id; SpriteId sprite; uint16_t w, h, row, col; };
    inline constexpr ItemDef ITEMS[] = {
        {ItemId::Pickaxe,     SpriteId::Tools,   16, 16,  97, 304},
        {ItemId::Sword,       SpriteId::Weapons, 16, 16,  16,  48},
        {ItemId::WateringCan, SpriteId::Tools,   16, 11, 228,  32},
        {ItemId::Shovel,      SpriteId::Tools,   16, 16,  33, 304},
        {ItemId::Coal,        SpriteId::Ores,    16, 16, 108,  16},
        {ItemId::Lapis,       SpriteId::Ores,    16, 16, 108,  64},
        {ItemId::Silver,      SpriteId::Ores,    15, 15,  68, 145},
        {ItemId::Gold,        SpriteId::Ores,    16, 16, 108,  94},
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
    };

    // Farmer
    inline constexpr ClipSetInfo FARMER_SET{ SpriteId::FarmerBase, Dimentions::FARMER_COLS, Dimentions::FARMER_CELL_W, Dimentions::FARMER_CELL_H};
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

    constexpr LayerOffsets farmer_offsets(int frame) {
        for (const auto& o : FARMER_OFFSETS)
            if (o.frame == frame) return o.off;
        return {};
    }

    // TODO: cambiar sprite dependiendo del color de vaca y pollo
    // Cow
    inline constexpr ClipSetInfo COW_SET{SpriteId::BronwCow, Dimentions::COW_COLS,Dimentions::COW_CELL_W, Dimentions::COW_CELL_H};
    inline constexpr int COW_IDLE_MS = 400;
    inline constexpr int COW_WALK_MS = 150;

    inline constexpr ClipDef<AnimalState> COW_CLIPS[] = {
        clip_run(AnimalState::Idle, Direction::Down,  true, 0,  2, COW_IDLE_MS),
        clip_run(AnimalState::Idle, Direction::Right, true, 4,  2, COW_IDLE_MS),
        clip_run(AnimalState::Idle, Direction::Up,    true, 8,  2, COW_IDLE_MS),
        clip_run(AnimalState::Idle, Direction::Left,  true, 12, 2, COW_IDLE_MS),

        clip_run(AnimalState::Walking, Direction::Down,  true, 0,  4, COW_WALK_MS),
        clip_run(AnimalState::Walking, Direction::Right, true, 4,  4, COW_WALK_MS),
        clip_run(AnimalState::Walking, Direction::Up,    true, 8,  4, COW_WALK_MS),
        clip_run(AnimalState::Walking, Direction::Left,  true, 12, 4, COW_WALK_MS),
    };

    // Chicken
    inline constexpr ClipSetInfo CHICKEN_SET{ SpriteId::BronwChicken, Dimentions::CHICKEN_COLS, Dimentions::CHICKEN_CELL_W, Dimentions::CHICKEN_CELL_H};

    inline constexpr int CHICKEN_IDLE_MS = 400;
    inline constexpr int CHICKEN_WALK_MS = 120;

    inline constexpr int CHICKEN_LEFT_ROW  = 0;
    inline constexpr int CHICKEN_RIGHT_ROW = 1 * Dimentions::CHICKEN_COLS;
    inline constexpr int CHICKEN_FRONT_ROW = 6 * Dimentions::CHICKEN_COLS;
    inline constexpr int CHICKEN_BACK_ROW  = CHICKEN_FRONT_ROW;

    inline constexpr ClipDef<AnimalState> CHICKEN_CLIPS[] = {
        clip_run(AnimalState::Idle, Direction::Left,  true, CHICKEN_LEFT_ROW,  2, CHICKEN_IDLE_MS),
        clip_run(AnimalState::Idle, Direction::Right, true, CHICKEN_RIGHT_ROW, 2, CHICKEN_IDLE_MS),
        clip_run(AnimalState::Idle, Direction::Down,  true, CHICKEN_FRONT_ROW, 2, CHICKEN_IDLE_MS),
        clip_run(AnimalState::Idle, Direction::Up,    true, CHICKEN_BACK_ROW,  2, CHICKEN_IDLE_MS),

        clip_run(AnimalState::Walking, Direction::Left,  true, CHICKEN_LEFT_ROW,  4, CHICKEN_WALK_MS),
        clip_run(AnimalState::Walking, Direction::Right, true, CHICKEN_RIGHT_ROW, 4, CHICKEN_WALK_MS),
        clip_run(AnimalState::Walking, Direction::Down,  true, CHICKEN_FRONT_ROW, 4, CHICKEN_WALK_MS),
        clip_run(AnimalState::Walking, Direction::Up,    true, CHICKEN_BACK_ROW,  4, CHICKEN_WALK_MS),
    };

}