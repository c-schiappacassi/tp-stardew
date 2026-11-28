#pragma once

#include <cstdint>
#include <initializer_list>
#include <map>
#include <optional>
#include <utility>
#include <vector>

#include "dtos.h"
#include "protocol_types.h"
#include "../sprite.h"
#include "../utils.h"

namespace Dimentions {    
    constexpr uint16_t FARMER_CELL_W  = 16;
    constexpr uint16_t FARMER_CELL_H  = 32;
    constexpr uint16_t FARMER_FEET_Y  = 30;

    constexpr uint16_t COW_CELL_W     = 32;
    constexpr uint16_t COW_CELL_H     = 32;

    constexpr uint16_t CHICKEN_CELL_W = 16;
    constexpr uint16_t CHICKEN_CELL_H = 16;

    constexpr uint16_t ORE_SIZE       = 16;
    constexpr uint16_t CROP_CELL_W    = 16;
    constexpr uint16_t CROP_CELL_H    = 32;

    constexpr uint16_t SKELETON_CELL_W = 16;
    constexpr uint16_t SKELETON_CELL_H = 32;
    constexpr uint16_t SKELETON_FEET_Y = 30;

    constexpr uint16_t GOLEM_CELL_W = 16;
    constexpr uint16_t GOLEM_CELL_H = 24;
    constexpr uint16_t GOLEM_FEET_Y = 22;

    constexpr uint16_t BAT_CELL_W = 16;
    constexpr uint16_t BAT_CELL_H = 16;
    constexpr uint16_t BAT_CELL_FRENZY_H = 17;
    constexpr uint16_t BAT_FEET_Y = 13;
    constexpr uint16_t BAT_FRENZY_FEET_Y = 15;

    constexpr uint16_t SKULL_CELL_W  = 16;
    constexpr uint16_t SKULL_CELL_H  = 16;
    constexpr uint16_t SKULL_HOVER_Y = 6;

    constexpr uint16_t WALL_FLAME_MAX_H = 19;

    constexpr uint16_t SPIKE_CELL_W = 16;
    constexpr uint16_t SPIKE_CELL_H = 16;

    constexpr uint16_t FLAME_CELL_W = 8;
    constexpr uint16_t FLAME_CELL_H = 8;

    constexpr uint16_t QUALITY_SIZE = 8;

    constexpr uint16_t CHEST_CELL_W = 16;
    constexpr uint16_t CHEST_CELL_H = 32;

    constexpr uint16_t JUNIMO_CELL_W = 16;
    constexpr uint16_t JUNIMO_CELL_H = 16;
}

namespace SpriteDefs {
    // Anclajes: el origen que se le pasa a draw es...
    // ...el centro de la base (pies)
    constexpr Sprite standing(SpriteId id, uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
        return Sprite(id, x, y, w, h, static_cast<int16_t>(-w / 2), static_cast<int16_t>(-h));
    }
    // ...la esquina del tile: centrado en los tiles que ocupa y apoyado abajo
    constexpr Sprite on_tile(SpriteId id, uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
        const int tiles_w = (w + TILE_TO_PIXEL - 1) / TILE_TO_PIXEL;
        const int tiles_h = (h + TILE_TO_PIXEL - 1) / TILE_TO_PIXEL;
        return Sprite(id, x, y, w, h, static_cast<int16_t>((tiles_w * TILE_TO_PIXEL - w) / 2),
                      static_cast<int16_t>(tiles_h * TILE_TO_PIXEL - h));
    }
    constexpr Sprite ore(uint16_t x, uint16_t y) { return standing(SpriteId::Ores, x, y, Dimentions::ORE_SIZE, Dimentions::ORE_SIZE); }
    constexpr Sprite crop(uint16_t x, uint16_t y) { return standing(SpriteId::Crops, x, y, Dimentions::CROP_CELL_W, Dimentions::CROP_CELL_H); }
    constexpr Sprite quality(uint16_t x, uint16_t y) { return standing(SpriteId::Cursors, x, y, Dimentions::QUALITY_SIZE, Dimentions::QUALITY_SIZE); }

    struct TextureDef { SpriteId id; const char* path; };

    inline constexpr TextureDef TEXTURES[] = {
        // placeholder para missing sprites
        {SpriteId::PlaceHolder, "placeholder.png"},

        // ui
        {SpriteId::MenuTiles,    "menus/menutiles.png"},
        {SpriteId::Cursors,      "menus/cursors.png"},
        {SpriteId::Emotes,      "misc/emotes.png"},
        {SpriteId::ChatBox,      "menus/chat_box.png"},
        {SpriteId::Minimap, "minimaps/placeholder_minimap.jpg"},
        {SpriteId::InfoSing, "menus/clock.png"},
        {SpriteId::NightSky, "menus/night_sky.png"},
        {SpriteId::DaySky,   "menus/day_sky.png"},
        {SpriteId::Junimo,   "misc/junimo.png"},

        // farmer
        {SpriteId::FarmerBase,   "characters/farmer_base.png"},
        {SpriteId::Hairstyles,   "characters/hairstyles.png"},
        {SpriteId::Pants,        "characters/pants.png"},
        {SpriteId::Shirts,       "characters/shirts.png"},
        {SpriteId::Tools,        "characters/tools.png"},
        {SpriteId::Weapons,      "characters/weapons.png"},

        // items y mas
        {SpriteId::Ores,         "caverns/mines.png"},
        {SpriteId::Crops,        "farm/crops/crops.png"},
        {SpriteId::Harvest , "farm/harvested_crops_seed_packets_minerals_food_equipment_miscellaneous_objects.png"},
        {SpriteId::Craftables, "misc/craftables.png"},

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
        {SpriteId::MinesTiles,       "caverns/mines_tiles.png"},
        {SpriteId::Furniture,       "farm/environment/furniture.png"},
        {SpriteId::LevelPortals,    "caverns/level_portals.png"},
        {SpriteId::TilledDirt,      "farm/environment/tilled_dirt.png"},
        {SpriteId::LightGlow,      "misc/sconce_light.png"},
    

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

        // interiors
        {SpriteId::PlayerHouseInterior, "farm/interior.png"},
        {SpriteId::MarniesInterior, "market/world/marnies_store_interior.png"},
        {SpriteId::PierresInterior, "market/world/pierre_store_interior.png"},
        {SpriteId::ClintsInterior, "market/world/clint_store_interior.png"},
        {SpriteId::RobinsInterior, "market/world/robin_store_interior.png"},
        {SpriteId::ComunityCenterInterior, "farm/interior.png"},
        {SpriteId::TownInteriors, "market/world/tilesets_town_interiors.png"},

        // npcs
        {SpriteId::Marnie, "market/npcs/marnie.png"},
        {SpriteId::Pierre , "market/npcs/pierre.png"},
        {SpriteId::Clint, "market/npcs/clint.png"},
        {SpriteId::Robin, "market/npcs/robin.png"},
    };

    // Ores
    inline const std::map<std::pair<ResourceType, ResourceState>, Sprite> ORES = {
        {{ResourceType::CoalRock,   ResourceState::Intact},   ore(0, 16)},
        {{ResourceType::CoalRock,   ResourceState::Broken},  ore(16, 16)},
        {{ResourceType::LapisRock,  ResourceState::Intact}, ore(193, 0)},
        {{ResourceType::LapisRock,  ResourceState::Broken}, ore(209, 0)},
        {{ResourceType::SilverRock, ResourceState::Intact}, ore(175, 106)},
        {{ResourceType::SilverRock, ResourceState::Broken}, ore(16, 0)},
        {{ResourceType::GoldRock,   ResourceState::Intact}, ore(192, 106)},
        {{ResourceType::GoldRock,   ResourceState::Broken}, ore(16, 16)},
        };

    // Crops
    inline const std::map<std::pair<CropType, CropState>, Sprite> CROPS = {
        {{CropType::Potato,     CropState::JustPlanted}, crop(160, 29)},
        {{CropType::Potato,     CropState::Sprouting},   crop(176, 29)},
        {{CropType::Potato,     CropState::Growing},     crop(208, 29)},
        {{CropType::Potato,     CropState::Ready},       crop(224, 29)},

        {{CropType::Pumpkin,    CropState::JustPlanted},   crop(0, 285)},
        {{CropType::Pumpkin,    CropState::Sprouting},    crop(48, 285)},
        {{CropType::Pumpkin,    CropState::Growing},      crop(80, 285)},
        {{CropType::Pumpkin,    CropState::Ready},       crop(96, 285)},

        {{CropType::Strawberry, CropState::JustPlanted}, crop(144, 318)},
        {{CropType::Strawberry, CropState::Sprouting},   crop(192, 318)},
        {{CropType::Strawberry, CropState::Growing},     crop(240, 318)},
        {{CropType::Strawberry, CropState::Ready},       crop(224, 318)},

        {{CropType::Scallion,   CropState::JustPlanted},  crop(16, 61)},
        {{CropType::Scallion,   CropState::Sprouting},    crop(48, 61)},
        {{CropType::Scallion,   CropState::Growing},      crop(64, 61)},
        {{CropType::Scallion,   CropState::Ready},        crop(80, 61)},
        };

    // Quality
    inline const std::map<Quality, Sprite> QUALITIES = {
        {Quality::Good,   quality(330, 392)},
        {Quality::Excellent,   quality(338, 392)},
        {Quality::Incredible,   quality(346, 392)},
        {Quality::Perfect,   quality(354, 392)},
    };

    // Map background
    inline const std::map<InteriorId, Sprite> INTERIORS = {
        {InteriorId::PlayerHouse , {SpriteId::PlayerHouseInterior, 0, 0, 272, 272}},
        {InteriorId::PierresShop , {SpriteId::PierresInterior, 0, 0, 288, 304}},
        {InteriorId::MarniesShop , {SpriteId::MarniesInterior, 0, 0, 176, 240}},
        {InteriorId::ClintsShop , {SpriteId::ClintsInterior, 0, 0, 224, 224}},
        {InteriorId::RobinsShop , {SpriteId::RobinsInterior, 0, 0, 176, 224}},
        {InteriorId::ComunityCenter , {SpriteId::ComunityCenterInterior, 0, 0, 272, 272}},
    };
    

    inline const std::map<NpcId, Sprite> NPCS = {
        {NpcId::Marnie , standing(SpriteId::Marnie, 0, 0, 16, 32)},
        {NpcId::Pierre , standing(SpriteId::Pierre, 0, 0, 16, 32)},
        {NpcId::Clint , standing(SpriteId::Clint, 0, 0, 16, 32)},
        {NpcId::Robin , standing(SpriteId::Robin, 0, 0, 16, 32)},
        };
    

    inline const std::map<TileTextureId, Sprite> TILES = {
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
        {TileTextureId::ConnctionStonePathGrassTopRightCorner,    {SpriteId::TownSpring, 32, 304, 16, 16}},
        {TileTextureId::ConnctionStonePathGrassBottomRightCorner, {SpriteId::TownSpring, 32, 336, 16, 16}},
        {TileTextureId::ConnctionStonePathGrassBottomLeftCorner,  {SpriteId::TownSpring, 0, 336, 16, 16}},
        {TileTextureId::ConnctionStonePathGrassRightSide,         {SpriteId::TownSpring, 32, 320, 16, 16}},
        {TileTextureId::TilledDirt,   {SpriteId::TilledDirt, 0,0, 16, 16}},
        {TileTextureId::WateredDirt,   {SpriteId::TilledDirt, 64,0, 16, 16}},

        {TileTextureId::DungeonFloor,                 {SpriteId::MinesTiles, 1216, 704, 16, 16}},
        {TileTextureId::DungeonDottedFloor,           {SpriteId::MinesTiles, 1104, 736, 16, 16}},
        {TileTextureId::DungeonPebbledFloor,          {SpriteId::MinesTiles, 1168, 720, 16, 16}},
        {TileTextureId::DungeonVoid,                  {SpriteId::MinesTiles, 1264, 640, 16, 16}},
        {TileTextureId::DungeonLeftWall,              {SpriteId::MinesTiles, 1088, 704, 16, 64}},
        {TileTextureId::DungeonRightWall,             {SpriteId::MinesTiles, 1264, 736, 16, 64}},
        {TileTextureId::DungeonBottom,                {SpriteId::MinesTiles, 1120, 784, 32, 16}},
        {TileTextureId::DungeonBottomLeftCorner1,     {SpriteId::MinesTiles, 1088, 768, 16, 16}},
        {TileTextureId::DungeonBottomRightCorner1,    {SpriteId::MinesTiles, 1248, 768, 16, 16}},
        {TileTextureId::DungeonInnerCornerTopRight,   {SpriteId::MinesTiles, 1104, 768, 16, 16}},
        {TileTextureId::DungeonInnerCornerTopLeft,    {SpriteId::MinesTiles, 1232, 768, 16, 16}},
    };

    inline constexpr bool is_wall_decor(DecorId id) {
        return id == DecorId::WallSpiderWeb || id == DecorId::WallLantern || id == DecorId::SmallWallLantern;
    }

    inline constexpr bool is_over_dungeon_floor(TileTextureId id) {
        switch (id) {
            case TileTextureId::DungeonLeftWall:
            case TileTextureId::DungeonRightWall:
            case TileTextureId::DungeonBottom:
            case TileTextureId::DungeonBottomLeftCorner1:
            case TileTextureId::DungeonBottomRightCorner1:
            case TileTextureId::DungeonInnerCornerTopRight:
            case TileTextureId::DungeonInnerCornerTopLeft:
                return true;
            default:
                return false;
        }
    }

    inline const std::map<DecorId, Sprite> DECOR = {
        {DecorId::MapleTree,         on_tile(SpriteId::Trees, 0, 0, 48, 97)},
        {DecorId::PineTree,          on_tile(SpriteId::Trees, 0, 320, 48, 97)},
        {DecorId::MushroomTree,      on_tile(SpriteId::Mushrooms, 0, 14, 48, 80)},
        {DecorId::PinkTree,          on_tile(SpriteId::Trees, 292, 160, 48, 97)},
        {DecorId::YellowTree,        on_tile(SpriteId::Trees, 292, 480, 48, 97)},
        {DecorId::BlueTree,          on_tile(SpriteId::Trees, 438, 1120, 48, 97)},
        {DecorId::PinkFlowerGrass,   on_tile(SpriteId::GrassDecor, 0, 0, 15, 20)},
        {DecorId::OrangeFlowerGrass, on_tile(SpriteId::GrassDecor, 0, 20, 15, 20)},
        {DecorId::OrangeGrass,       on_tile(SpriteId::GrassDecor, 0, 40, 15, 20)},
        {DecorId::VelvetGrass,       on_tile(SpriteId::GrassDecor, 15, 100, 15, 20)},
        {DecorId::BlueGrass,         on_tile(SpriteId::GrassDecor, 15, 160, 15, 20)},
        {DecorId::PlainGrass,        on_tile(SpriteId::GrassDecor, 0, 60, 15, 20)},
        {DecorId::RedGrass,          on_tile(SpriteId::GrassDecor, 15, 40, 15, 20)},
        {DecorId::SimpleBush,        on_tile(SpriteId::Bushes, 0, 9, 32, 36)},
        {DecorId::RedBBerryBush,     on_tile(SpriteId::Bushes, 32, 9, 32, 36)},
        {DecorId::PinkBush,          on_tile(SpriteId::Bushes, 32, 95, 32, 32)},
        {DecorId::PurpleBerryBush,   on_tile(SpriteId::Bushes, 32, 57, 32, 36)},
        {DecorId::BlueBush,          on_tile(SpriteId::Bushes, 64, 57, 32, 36)},
        {DecorId::BigRedBush,        on_tile(SpriteId::Bushes, 48, 132, 48, 43)},
        {DecorId::StreatLamp,        on_tile(SpriteId::TownSpring, 176, 448, 16, 64)},
        {DecorId::FenceFront,        on_tile(SpriteId::TownSpring, 16, 449, 16, 32)},
        {DecorId::FenceBack,         on_tile(SpriteId::TownSpring, 16, 400, 16, 32)},
        {DecorId::PinkFlower,        on_tile(SpriteId::OutdoorsSpring, 112, 160, 16, 16)},
        {DecorId::BlueFlower,        on_tile(SpriteId::OutdoorsSpring, 112, 192, 16, 16)},
        {DecorId::WeirdGreenPlant,   on_tile(SpriteId::CavernPlants, 0, 0, 16, 24)},
        {DecorId::SmallFern,         on_tile(SpriteId::CavernPlants, 16, 0, 16, 24)},
        {DecorId::BigFern,           on_tile(SpriteId::CavernPlants, 32, 0, 16, 24)},
        {DecorId::PinkMushrom,       on_tile(SpriteId::CavernPlants, 96, 0, 16, 24)},
        {DecorId::YellowMushrom,     on_tile(SpriteId::CavernPlants, 128, 0, 16, 24)},
        {DecorId::BabyRedMushrom,    on_tile(SpriteId::Mushrooms, 0, 128, 16, 16)},
        {DecorId::BigRedMushroom,    on_tile(SpriteId::Mushrooms, 0, 102, 16, 25)},
        {DecorId::DungeonPilar,      on_tile(SpriteId::DungeonWallTiles, 0, 384, 16, 48)},
        {DecorId::DungeonTorch,      on_tile(SpriteId::DungeonWallTiles, 64, 388, 16, 27)},
        {DecorId::DungeonChain,      on_tile(SpriteId::DungeonWallTiles, 64, 416, 16, 32)},
        {DecorId::DungeonWindow,     on_tile(SpriteId::DungeonWallTiles, 64, 448, 16, 32)},
        {DecorId::DungeonWallCrack,  on_tile(SpriteId::DungeonWallTiles, 64, 480, 16, 32)},
        {DecorId::SpiderWeb,         on_tile(SpriteId::GrassDecor, 15, 140, 16, 20)},
        {DecorId::CornerSpiderWeb,   on_tile(SpriteId::GrassDecor, 31, 140, 16, 20)},
        {DecorId::PottedPlants,      on_tile(SpriteId::TownSpring, 368, 476, 47, 36)},
        {DecorId::SmallBlueFlower,   on_tile(SpriteId::TownSpring, 353, 432, 16, 16)},
        {DecorId::SignPost,          on_tile(SpriteId::OutdoorsSpring, 160, 256, 16, 32)},
        {DecorId::FenceSideLeft,     on_tile(SpriteId::TownSpring, 0, 432, 16, 16)},
        {DecorId::FenceSideRight,    on_tile(SpriteId::TownSpring, 32, 432, 16, 16)},
        {DecorId::SmallBench,        on_tile(SpriteId::TownSpring, 32, 154, 32, 22)},
        {DecorId::PottedFern,        on_tile(SpriteId::TownSpring, 16, 145, 16, 16)},
        {DecorId::ChairFacingRight,  on_tile(SpriteId::TownSpring, 128, 9, 16, 38)},
        {DecorId::Mailbox,           on_tile(SpriteId::TownSpring, 146, 0, 12, 32)},
        {DecorId::TrashCan,          on_tile(SpriteId::TownSpring, 224, 26, 16, 22)},
        {DecorId::ChairFacingLeft,   on_tile(SpriteId::TownSpring, 352, 9, 15, 38)},
        {DecorId::FlowerBox,         on_tile(SpriteId::TownSpring, 368, 9, 16, 21)},
        {DecorId::PottedFlower,      on_tile(SpriteId::TownSpring, 384, 16, 16, 32)},
        {DecorId::Table,             on_tile(SpriteId::TownSpring, 432, 526, 48, 34)},
        {DecorId::WallSpiderWeb,     on_tile(SpriteId::MinesTiles, 1056, 704, 32, 32)},
        {DecorId::WallLantern,       on_tile(SpriteId::MinesTiles, 1040, 640, 32, 32)},
        {DecorId::SmallWallLantern,  on_tile(SpriteId::MinesTiles, 1040, 672, 16, 16)},
        };

    inline const std::map<std::pair<FurnitureId, Direction>, FlippableSprite> FURNITURE = [] {
        struct Def { FurnitureId id; Direction dir; Sprite sprite; };
        const Def defs[] = {
    {FurnitureId::Chair,               Direction::Down,   standing(SpriteId::Furniture, 0, 0, 16, 32)},
            {FurnitureId::Chair,               Direction::Up,     standing(SpriteId::Furniture, 32, 0, 16, 32)},
            {FurnitureId::Chair,               Direction::Right,  standing(SpriteId::Furniture, 16, 0, 16, 32)},
            {FurnitureId::Table,               Direction::None,   standing(SpriteId::Furniture, 0, 569, 32, 39)},
            {FurnitureId::Bed,                 Direction::None,   standing(SpriteId::Furniture, 0, 1032, 32, 55)},
            {FurnitureId::BedSheet,            Direction::None,   standing(SpriteId::Furniture, 32, 1032, 32, 55)},
            {FurnitureId::Bonsai,              Direction::None,   standing(SpriteId::Furniture, 514, 633, 32, 48)},
            {FurnitureId::PottedTree,          Direction::None,   standing(SpriteId::Furniture, 514, 585, 32, 48)},
            {FurnitureId::BlueArmCouch,        Direction::Down,   standing(SpriteId::Furniture, 0, 144, 32, 32)},
            {FurnitureId::BlueArmCouch,        Direction::Right,  standing(SpriteId::Furniture, 32, 144, 32, 32)},
            {FurnitureId::BlueArmCouch,        Direction::Up,     standing(SpriteId::Furniture, 64, 144, 32, 32)},
            {FurnitureId::Tv,                  Direction::None,   standing(SpriteId::Furniture, 259, 832, 26, 32)},
            {FurnitureId::BeigeRug,            Direction::None,   standing(SpriteId::Furniture, 229, 871, 24, 20)},
            {FurnitureId::ClockFirePlace,      Direction::None,   standing(SpriteId::Furniture, 160, 938, 32, 70)},
            {FurnitureId::BlueRug,             Direction::None,   standing(SpriteId::Furniture, 149, 867, 37, 28)},
            {FurnitureId::CatFirePlace,        Direction::None,   standing(SpriteId::Furniture, 128, 925, 32, 51)},
            {FurnitureId::SimpleFirePlace,     Direction::None,   standing(SpriteId::Furniture, 0, 896, 32, 80)},
            {FurnitureId::BlueJunimoPushee,    Direction::None,   standing(SpriteId::Furniture, 16, 880, 16, 16)},
            {FurnitureId::GreenJunimoPushee,   Direction::None,   standing(SpriteId::Furniture, 0, 880, 16, 16)},
            {FurnitureId::YellowJunimoPushee,  Direction::None,   standing(SpriteId::Furniture, 32, 880, 16, 16)},
            {FurnitureId::PinkJunimoPushee,    Direction::None,   standing(SpriteId::Furniture, 48, 880, 16, 16)},
            {FurnitureId::SimpleLamp,          Direction::None,   standing(SpriteId::Furniture, 49, 733, 16, 34)},
            {FurnitureId::BookShelf,           Direction::None,   standing(SpriteId::Furniture, 80, 640, 32, 48)},
            {FurnitureId::SquareLamp,          Direction::None,   standing(SpriteId::Furniture, 80, 729, 16, 39)},
            {FurnitureId::RoundTable,          Direction::None,   standing(SpriteId::Furniture, 224, 570, 32, 38)},
            {FurnitureId::GreenStool,          Direction::None,   standing(SpriteId::Furniture, 480, 47, 16, 17)},
            {FurnitureId::BrownSofa,           Direction::Down,   standing(SpriteId::Furniture, 0, 256, 48, 32)},
            {FurnitureId::BrownSofa,           Direction::Right,  standing(SpriteId::Furniture, 48, 256, 32, 48)},
            {FurnitureId::BrownSofa,           Direction::Up,     standing(SpriteId::Furniture, 80, 256, 48, 32)},
            {FurnitureId::BrownArmCouch,       Direction::Down,   standing(SpriteId::Furniture, 384, 144, 32, 32)},
            {FurnitureId::BrownArmCouch,       Direction::Right,  standing(SpriteId::Furniture, 416, 144, 32, 32)},
            {FurnitureId::BrownArmCouch,       Direction::Up,     standing(SpriteId::Furniture, 448, 144, 32, 32)},

            // shop furniture
            {FurnitureId::PierreShopShelf,     Direction::Down,   standing(SpriteId::TownInteriors, 416, 464, 96, 48)},
            {FurnitureId::PierreShopShelf,     Direction::Up,     standing(SpriteId::TownInteriors, 416, 464, 96, 48)},
            {FurnitureId::PierreShopShelf,     Direction::Left,   standing(SpriteId::TownInteriors, 160, 928, 32, 96)},
            {FurnitureId::PierreShopShelf,     Direction::Right,  standing(SpriteId::TownInteriors, 128, 928, 32, 96)},
            {FurnitureId::PierreCashRegister,  Direction::None,   standing(SpriteId::TownInteriors, 304, 480, 96, 32)},
            {FurnitureId::PierreProductBin,    Direction::None,   standing(SpriteId::TownInteriors, 416, 428, 32, 36)},
            {FurnitureId::MarnieGrass,         Direction::None,   standing(SpriteId::TownInteriors, 336, 344, 16, 24)},
            {FurnitureId::CashRegister,        Direction::None,   standing(SpriteId::TownInteriors, 336, 452, 48, 28)},
            {FurnitureId::ClintsChair,         Direction::None,   standing(SpriteId::TownInteriors, 143, 243, 16, 28)},
            {FurnitureId::RobinCashRegister,   Direction::None,   standing(SpriteId::TownInteriors, 336, 413, 48, 28)},
            {FurnitureId::RobinPlantTable,     Direction::None,   standing(SpriteId::TownInteriors, 305, 1386, 16, 38)},
            };
        std::map<std::pair<FurnitureId, Direction>, FlippableSprite> m;
        for (const Def& d : defs) {
            if (d.dir == Direction::None) {
                for (Direction dir : {Direction::None, Direction::Up, Direction::Down, Direction::Left, Direction::Right})
                    m.emplace(std::pair{d.id, dir}, FlippableSprite{d.sprite});
            } else {
                m.insert_or_assign({d.id, d.dir}, FlippableSprite{d.sprite});
                if (d.dir == Direction::Right) m.emplace(std::pair{d.id, Direction::Left}, FlippableSprite{d.sprite, true});
            }
        }
        return m;
    }();

    inline const std::map<BuildingId, Sprite> BUILDINGS = {
        {BuildingId::ComunityCenter, on_tile(SpriteId::MarketBuildings, 0, 160, 192, 160)},
        {BuildingId::PierresShop,    on_tile(SpriteId::MarketBuildings, 0, 0, 161, 160)},
        {BuildingId::ClintsShop,     on_tile(SpriteId::MarketBuildings, 161, 0, 130, 127)},
        {BuildingId::MarniesShop,    on_tile(SpriteId::MarketBuildings, 291, 0, 128, 112)},
        {BuildingId::RobinsShop,     on_tile(SpriteId::MarketBuildings, 192, 127, 157, 193)},
        {BuildingId::FarmHouse,      on_tile(SpriteId::FarmBuildings, 157, 268, 144, 145)},
        {BuildingId::StoneCabin,     on_tile(SpriteId::Cabins, 80, 336, 80, 112)},
        {BuildingId::WoodCabin,      on_tile(SpriteId::Cabins, 80, 0, 80, 112)},
        {BuildingId::BrickCabin,     on_tile(SpriteId::Cabins, 80, 112, 80, 112)},
        };

    // Frame
    inline const std::map<FramePieceId, Sprite> FRAMES = {
        {FramePieceId::StoneCliftTop,               on_tile(SpriteId::OutdoorsSpring, 272, 288, 32, 64)},
        {FramePieceId::StoneCliftTopLeftCorner,     on_tile(SpriteId::OutdoorsSpring, 336, 176, 16, 64)},
        {FramePieceId::StoneCliftTopRightCorner,    on_tile(SpriteId::OutdoorsSpring, 384, 176, 16, 64)},
        {FramePieceId::StoneCliftDropRight,         on_tile(SpriteId::OutdoorsSpring, 464, 400, 64, 64)},
        {FramePieceId::StoneCliftDropLeft,          on_tile(SpriteId::OutdoorsSpring, 464, 464, 64, 64)},
        {FramePieceId::StoneCliftLeftSide,          on_tile(SpriteId::OutdoorsSpring, 304, 224, 16, 48)},
        {FramePieceId::StoneCliftRightSide,         on_tile(SpriteId::OutdoorsSpring, 256, 224, 16, 48)},
        {FramePieceId::StoneCliftBottom,            on_tile(SpriteId::OutdoorsSpring, 208, 256, 16, 16)},
        {FramePieceId::StoneCliftBottomLeftCorner,  on_tile(SpriteId::OutdoorsSpring, 192, 256, 16, 16)},
        {FramePieceId::StoneCliftBottomRightCorner, on_tile(SpriteId::OutdoorsSpring, 240, 256, 16, 16)},

        {FramePieceId::DungeonTopWall,              on_tile(SpriteId::MinesTiles, 1168, 640, 64, 64)},
        {FramePieceId::DungeonTopLeftCorner1,       on_tile(SpriteId::MinesTiles, 1088, 664, 16, 40)},
        {FramePieceId::DungeonTopLeftCorner2,       on_tile(SpriteId::MinesTiles, 1104, 640, 32, 80)},
        {FramePieceId::DungeonTopRightCorner1,      on_tile(SpriteId::MinesTiles, 1232, 656, 32, 80)},
        {FramePieceId::DungeonTopRightCorner2,      on_tile(SpriteId::MinesTiles, 1264, 692, 16, 44)},
        {FramePieceId::DungeonBottomLeftCorner2,    on_tile(SpriteId::MinesTiles, 1104, 768, 16, 32)},
        {FramePieceId::DungeonBottomRightCorner2,   on_tile(SpriteId::MinesTiles, 1232, 768, 16, 32)},
        {FramePieceId::DungeonTopWallEndLeft,       on_tile(SpriteId::MinesTiles, 1151, 640, 16, 64)},
        {FramePieceId::DungeonTopWallEndRight,      on_tile(SpriteId::MinesTiles, 1137, 640, 16, 64)},
        {FramePieceId::DungeonLantern,              on_tile(SpriteId::MinesTiles, 1040, 672, 16, 16)},
        };

    inline const std::map<PortalId, Sprite> PORTALS = {
        {PortalId::FarmToMine,  standing(SpriteId::OutdoorsSpring, 608, 160, 48, 48)},
        {PortalId::ToNextLevel,  standing(SpriteId::LevelPortals, 0, 0, 16, 16)},
        {PortalId::MineToFarm,  standing(SpriteId::LevelPortals, 0, 16, 16, 64)},
        };

    // Items
    inline const std::map<ItemId, Sprite> ITEMS = {
        {ItemId::Pickaxe, {SpriteId::Tools, 304, 97, 16, 16}},
        {ItemId::Sword, {SpriteId::Weapons, 48, 16, 16, 16}},
        {ItemId::WateringCan, {SpriteId::Tools, 32, 228, 16, 11}},
        {ItemId::Shovel, {SpriteId::Tools, 304, 33, 16, 16}},
        {ItemId::Coal, {SpriteId::Ores, 17, 108, 17, 17}},
        {ItemId::Lapis, {SpriteId::Ores, 128, 50, 17, 17}},
        {ItemId::Silver, {SpriteId::Ores, 63, 107, 17, 17}},
        {ItemId::Gold, {SpriteId::Ores, 94, 108, 16, 16}},
        {ItemId::Milk, {SpriteId::Harvest, 290, 112, 13, 16}},
        {ItemId::Egg, {SpriteId::Harvest, 224, 113, 16, 16}},
        {ItemId::StrawberrySeed, {SpriteId::Harvest, 256, 320, 16, 16}},
        {ItemId::ScallionSeed, {SpriteId::Harvest, 176, 320, 16, 16}},
        {ItemId::PotatoSeed, {SpriteId::Harvest, 304, 304, 16, 16}},
        {ItemId::PumpkinSeed, {SpriteId::Harvest, 160, 320, 16, 16}},
        {ItemId::Strawberry, {SpriteId::Harvest, 256, 256, 16, 16}},
        {ItemId::Scallion, {SpriteId::Harvest, 240, 256, 16, 16}},
        {ItemId::Potato, {SpriteId::Harvest, 0, 128, 16, 16}},
        {ItemId::Pumpkin, {SpriteId::Harvest, 192, 176, 16, 16}},
        {ItemId::Chest, {SpriteId::Craftables, 32, 512, 16, 32}},
        {ItemId::Chair, {SpriteId::Furniture, 0, 0, 16, 32}},
        {ItemId::Table, {SpriteId::Furniture, 0, 569, 32, 39}},
        {ItemId::Bed, {SpriteId::Furniture, 0, 1032, 32, 55}},
        {ItemId::BedSheet, {SpriteId::Furniture, 32, 1032, 32, 55}},
        {ItemId::Bonsai, {SpriteId::Furniture, 514, 633, 32, 48}},
        {ItemId::PottedTree, {SpriteId::Furniture, 514, 585, 32, 48}},
        {ItemId::BlueArmCouch, {SpriteId::Furniture, 0, 144, 32, 32}},
        {ItemId::Tv, {SpriteId::Furniture, 259, 832, 26, 32}},
        {ItemId::BeigeRug, {SpriteId::Furniture, 229, 871, 24, 20}},
        {ItemId::ClockFirePlace, {SpriteId::Furniture, 160, 938, 32, 70}},
        {ItemId::BlueRug, {SpriteId::Furniture, 149, 867, 37, 28}},
        {ItemId::CatFirePlace, {SpriteId::Furniture, 128, 925, 32, 51}},
        {ItemId::SimpleFirePlace, {SpriteId::Furniture, 0, 896, 32, 80}},
        {ItemId::BlueJunimoPushee, {SpriteId::Furniture, 16, 880, 16, 16}},
        {ItemId::GreenJunimoPushee, {SpriteId::Furniture, 0, 880, 16, 16}},
        {ItemId::YellowJunimoPushee, {SpriteId::Furniture, 32, 880, 16, 16}},
        {ItemId::PinkJunimoPushee, {SpriteId::Furniture, 48, 880, 16, 16}},
        {ItemId::SimpleLamp, {SpriteId::Furniture, 49, 733, 16, 34}},
        {ItemId::BookShelf, {SpriteId::Furniture, 80, 640, 32, 48}},
        {ItemId::SquareLamp, {SpriteId::Furniture, 80, 729, 16, 39}},
        {ItemId::RoundTable, {SpriteId::Furniture, 224, 570, 32, 38}},
        {ItemId::GreenStool, {SpriteId::Furniture, 480, 47, 16, 17}},
        {ItemId::BrownSofa, {SpriteId::Furniture, 0, 256, 48, 32}},
        {ItemId::BrownArmCouch, {SpriteId::Furniture, 384, 144, 32, 32}},
        };

    inline const std::map<EmoteId, Sprite> EMOTES = {
        {EmoteId::Alert, standing(SpriteId::Emotes, 0, 64, 16, 16)},
        };

    // UI
    inline const std::map<UiSpriteId, Sprite> UI = {
        {UiSpriteId::HotbarBar, {SpriteId::MenuTiles, 0, 256, 60, 60}},
        {UiSpriteId::HotbarSlot, {SpriteId::MenuTiles, 64, 128, 64, 64}},
        {UiSpriteId::HotbarSlotOutline, {SpriteId::MenuTiles, 128, 128, 64, 64}},
        {UiSpriteId::HotbarSlotSelected, {SpriteId::MenuTiles, 0, 896, 64, 64}},
        {UiSpriteId::EnergyBar, {SpriteId::Cursors, 256, 408, 12, 56}},
        {UiSpriteId::HealthBar, {SpriteId::Cursors, 268, 408, 12, 56}},
        {UiSpriteId::ChatBox, {SpriteId::ChatBox, 0, 14, 75, 61}},
        {UiSpriteId::ChatTypeBox, {SpriteId::ChatBox, 0, 0, 75, 14}},
        {UiSpriteId::DialogBox, {SpriteId::MenuTiles, 0, 256, 60, 60}},
        {UiSpriteId::OkButton, {SpriteId::Cursors, 128, 256, 64, 64}},
        {UiSpriteId::CancelButton, {SpriteId::Cursors, 192, 256, 64, 64}},
        {UiSpriteId::PlayerHead, {SpriteId::FarmerBase, 0, 0, 16, 15}},
        {UiSpriteId::InfoSing, {SpriteId::InfoSing, 0, 0, 72, 59}},
        {UiSpriteId::InfoSingArrow, {SpriteId::InfoSing, 72, 0, 7, 19}},
        {UiSpriteId::NightSky, {SpriteId::NightSky, 0, 0, 480, 256}},
        };

    // Clips
    struct FrameRect { uint16_t x, y, w, h; int ms; };

    constexpr int MAX_FRAMES = 8;

    template <class State>
    struct ClipDef {
        State state;
        Direction dir;
        bool loop;
        int count;
        FrameRect frames[MAX_FRAMES];
        std::optional<ItemId> item = std::nullopt;
    };

    template <class State>
    constexpr ClipDef<State> clip(State s, Direction d, bool loop, std::initializer_list<FrameRect> f) {
        ClipDef<State> c{s, d, loop, 0, {}};
        for (const auto& fr : f) c.frames[c.count++] = fr;
        return c;
    }

    template <class State>
    constexpr ClipDef<State> clip(ItemId item, State s, Direction d, bool loop, std::initializer_list<FrameRect> f) {
        ClipDef<State> c{s, d, loop, 0, {}, item};
        for (const auto& fr : f) c.frames[c.count++] = fr;
        return c;
    }

    struct ClipSheet {
        SpriteId sprite;
        int16_t baseline = 0;
    };

    // Farmer
    inline constexpr ClipSheet FARMER_SHEET{SpriteId::FarmerBase, Dimentions::FARMER_CELL_H - Dimentions::FARMER_FEET_Y};
    inline constexpr bool FARMER_MIRROR_LEFT = true;

    inline constexpr ClipDef<PlayerState> FARMER_CLIPS[] = {
        clip(PlayerState::Idle, Direction::Down, false, {{0, 0, 16, 32, 200}}),
        clip(PlayerState::Idle, Direction::Right, false, {{0, 32, 16, 32, 200}}),
        clip(PlayerState::Idle, Direction::Up, false, {{0, 64, 16, 32, 200}}),

        clip(PlayerState::Sleeping, Direction::Down, true, {{64, 64, 16, 32, 1000}}),
        clip(PlayerState::Sleeping, Direction::Right, true, {{64, 64, 16, 32, 1000}}),
        clip(PlayerState::Sleeping, Direction::Up, true, {{64, 64, 16, 32, 1000}}),

        clip(PlayerState::Unconscious, Direction::Down, false, {{64, 0, 16, 32, 150}, {80, 0, 16, 32, 300}}),
        clip(PlayerState::Unconscious, Direction::Right, false, {{64, 0, 16, 32, 150}, {80, 0, 16, 32, 300}}),
        clip(PlayerState::Unconscious, Direction::Up, false, {{64, 0, 16, 32, 150}, {80, 0, 16, 32, 300}}),

        clip(PlayerState::LookingChest, Direction::Down, false, {{0, 0, 16, 32, 200}}),
        clip(PlayerState::LookingChest, Direction::Right, false, {{0, 32, 16, 32, 200}}),
        clip(PlayerState::LookingChest, Direction::Up, false, {{0, 64, 16, 32, 200}}),

        clip(PlayerState::Walking, Direction::Down, true, {{16, 0, 16, 32, 200}, {0, 0, 16, 32, 200}, {32, 0, 16, 32, 200}, {0, 0, 16, 32, 200}}),
        clip(PlayerState::Walking, Direction::Right, true, {{16, 32, 16, 32, 200}, {0, 32, 16, 32, 200}, {32, 32, 16, 32, 200}, {0, 32, 16, 32, 200}}),
        clip(PlayerState::Walking, Direction::Up, true, {{16, 64, 16, 32, 200}, {0, 64, 16, 32, 200}, {32, 64, 16, 32, 200}, {0, 64, 16, 32, 200}}),

        clip(PlayerState::UsingTool, Direction::Down, false, {{0, 352, 16, 32, 100}, {16, 352, 16, 32, 100}, {32, 352, 16, 32, 100}, {48, 352, 16, 32, 100}, {64, 352, 16, 32, 100}}),
        clip(PlayerState::UsingTool, Direction::Right, false, {{0, 256, 16, 32, 100}, {16, 256, 16, 32, 100}, {32, 256, 16, 32, 100}, {48, 256, 16, 32, 100}, {64, 256, 16, 32, 100}}),
        clip(PlayerState::UsingTool, Direction::Up, false, {{0, 192, 16, 32, 100}, {16, 192, 16, 32, 100}, {32, 192, 16, 32, 100}, {48, 192, 16, 32, 100}, {64, 192, 16, 32, 100}}),

        clip(ItemId::WateringCan, PlayerState::UsingTool, Direction::Down, false, {{0, 288, 16, 32, 150}, {16, 288, 16, 32, 150}, {16, 288, 16, 32, 150}, {16, 288, 16, 32, 150}, {0, 288, 16, 32, 150}}),
        clip(ItemId::WateringCan, PlayerState::UsingTool, Direction::Right, false, {{64, 288, 16, 32, 150}, {80, 288, 16, 32, 150}, {80, 288, 16, 32, 150}, {80, 288, 16, 32, 150}, {64, 288, 16, 32, 150}}),
        clip(ItemId::WateringCan, PlayerState::UsingTool, Direction::Up, false, {{32, 320, 16, 32, 150}, {48, 320, 16, 32, 150}, {48, 320, 16, 32, 150}, {48, 320, 16, 32, 150}, {32, 320, 16, 32, 150}}),

        clip(ItemId::Sword, PlayerState::UsingTool, Direction::Down, false, {{0, 128, 16, 32, 100}, {16, 128, 16, 32, 70}, {32, 128, 16, 32, 60}, {48, 128, 16, 32, 60}, {80, 128, 16, 32, 150}}),
        clip(ItemId::Sword, PlayerState::UsingTool, Direction::Right, false, {{0, 160, 16, 32, 100}, {16, 160, 16, 32, 70}, {32, 160, 16, 32, 60}, {48, 160, 16, 32, 60}, {80, 160, 16, 32, 150}}),
    };

    struct FarmerOffsetDef { uint16_t x, y; LayerOffsets off; };

    inline constexpr FarmerOffsetDef FARMER_OFFSETS[] = {
        // x - y - hair - shirt - pants
        // Idle / walking down
        {  0,   0, {{ 0,  0}, { 0,  0}, { 0, 0}}},
        { 16,   0, {{ 0,  1}, { 0,  1}, { 0, 0}}},
        { 32,   0, {{ 0,  1}, { 0,  1}, { 0, 0}}},

        // Idle / walking side
        {  0,  32, {{ 0,  -1}, { 0,  0}, { 0, 0}}},
        { 16,  32, {{ 0,  0}, { 0,  1}, { 0, 0}}},
        { 32,  32, {{ 0,  0}, { 0,  1}, { 0, 0}}},

        // Idle / walking up
        {  0,  64, {{ 0,  0}, { 0,  0}, { 0, 0}}},
        { 16,  64, {{ 0,  1}, { 0,  0}, { 0, 0}}},
        { 32,  64, {{ 0,  1}, { 0,  0}, { 0, 0}}},

        // Sleeping
        { 64,  64, {{ 0,  1}, { 0,  1}, { 0, 0}}},

        // Unconscious
        { 64,   0, {{ 0,  4}, { 0,  4}, { 0, 1}}},
        { 80,   0, {{ 0,  5}, { 0,  5}, { 0, 1}}},

        // Using tool up
        {  0, 192, {{  0,  0}, {  0,  0}, {0, 0}}},
        { 16, 192, {{  0,  0}, {  0,  0}, {0, 0}}},
        { 32, 192, {{  0, -1}, {  0, -1}, {0, 0}}},
        { 48, 192, {{  0, -1}, {  0, -1}, {0, 0}}},
        { 64, 192, {{  0,  0}, {  0,  0}, {0, 0}}},

        // Using tool side
        {  0, 256, {{ -1,  -1}, { -1,  0}, {0, 0}}},
        { 16, 256, {{  0, -2}, {  0, -1}, {0, 0}}},
        { 32, 256, {{  0, -3}, {  0, -2}, {0, 0}}},
        { 48, 256, {{  0, -4}, {  0, -3}, {0, 0}}},
        { 64, 256, {{  0, -3}, {  0, -2}, {0, 0}}},

        // Using tool down
        {  0, 352, {{  0,  1}, {  0,  1}, {0, 0}}},
        { 16, 352, {{  0,  0}, {  0,  0}, {0, 0}}},
        { 32, 352, {{  0, -1}, {  0, -1}, {0, 0}}},
        { 48, 352, {{  0, -2}, {  0, -2}, {0, 0}}},
        { 64, 352, {{  0,  0}, {  0,  0}, {0, 0}}},

        // Watering can down
        {  0, 288, {{0, 4}, {0, 4}, {0, 0}}},
        { 16, 288, {{0, 3}, {0, 3}, {0, 0}}},
        // Watering can side
        { 64, 288, {{ 0, 1}, { 0, 2}, {0, 0}}},
        { 80, 288, {{-1, 0}, {-1, 1}, {0, 0}}},
        // Watering can up
        { 32, 320, {{0, 3}, {0, 3}, {0, 0}}},
        { 48, 320, {{0, 1}, {0, 1}, {0, 0}}},

        // Sword down
        {  0, 128, {{0, -1}, {0, -1}, {0, 0}}},
        { 16, 128, {{0, -1}, {0, -1}, {0, 0}}},
        { 32, 128, {{0,  1}, {0,  1}, {0, 0}}},
        { 48, 128, {{0,  1}, {0,  1}, {0, 0}}},
        { 80, 128, {{0,  3}, {0,  3}, {0, 0}}},
        // Sword side
        {  0, 160, {{0, -2}, {0, -1}, {0, 0}}},
        { 16, 160, {{0, -2}, {0, -1}, {0, 0}}},
        { 32, 160, {{0, -1}, {0,  0}, {0, 0}}},
        { 48, 160, {{0,  0}, {0,  1}, {0, 0}}},
        { 80, 160, {{0, -1}, {0,  0}, {0, 0}}},
    };

    constexpr LayerOffsets farmer_offsets(uint16_t x, uint16_t y) {
        for (const auto& o : FARMER_OFFSETS)
            if (o.x == x && o.y == y) return o.off;
        return {};
    }

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

    // Cow
    inline constexpr ClipSheet COW_SHEET{SpriteId::BronwCow};
    inline constexpr int COW_IDLE_MS = 400;
    inline constexpr int COW_WALK_MS = 150;

    inline constexpr ClipDef<AnimalState> COW_CLIPS[] = {
        clip(AnimalState::Idle, Direction::Down, true, {{0, 0, 32, 32, COW_IDLE_MS}}),
        clip(AnimalState::Idle, Direction::Right, true, {{0, 32, 32, 32, COW_IDLE_MS}}),
        clip(AnimalState::Idle, Direction::Up, true, {{0, 64, 32, 32, COW_IDLE_MS}}),

        clip(AnimalState::Walking, Direction::Down, true, {{0, 0, 32, 32, COW_WALK_MS}, {32, 0, 32, 32, COW_WALK_MS}, {64, 0, 32, 32, COW_WALK_MS}, {96, 0, 32, 32, COW_WALK_MS}}),
        clip(AnimalState::Walking, Direction::Right, true, {{0, 32, 32, 32, COW_WALK_MS}, {32, 32, 32, 32, COW_WALK_MS}, {64, 32, 32, 32, COW_WALK_MS}, {96, 32, 32, 32, COW_WALK_MS}}),
        clip(AnimalState::Walking, Direction::Up, true, {{0, 64, 32, 32, COW_WALK_MS}, {32, 64, 32, 32, COW_WALK_MS}, {64, 64, 32, 32, COW_WALK_MS}, {96, 64, 32, 32, COW_WALK_MS}}),
    };

    // Chicken
    inline constexpr ClipSheet CHICKEN_SHEET{SpriteId::BronwChicken};
    inline constexpr int CHICKEN_IDLE_MS = 400;
    inline constexpr int CHICKEN_WALK_MS = 120;

    inline constexpr ClipDef<AnimalState> CHICKEN_CLIPS[] = {
        clip(AnimalState::Idle, Direction::Down, true, {{0, 0, 16, 16, CHICKEN_IDLE_MS}}),
        clip(AnimalState::Idle, Direction::Right, true, {{0, 16, 16, 16, CHICKEN_IDLE_MS}}),
        clip(AnimalState::Idle, Direction::Up, true, {{0, 32, 16, 16, CHICKEN_IDLE_MS}}),

        clip(AnimalState::Walking, Direction::Down, true, {{0, 0, 16, 16, CHICKEN_WALK_MS}, {16, 0, 16, 16, CHICKEN_WALK_MS}, {32, 0, 16, 16, CHICKEN_WALK_MS}, {48, 0, 16, 16, CHICKEN_WALK_MS}}),
        clip(AnimalState::Walking, Direction::Right, true, {{0, 16, 16, 16, CHICKEN_WALK_MS}, {16, 16, 16, 16, CHICKEN_WALK_MS}, {32, 16, 16, 16, CHICKEN_WALK_MS}, {48, 16, 16, 16, CHICKEN_WALK_MS}}),
        clip(AnimalState::Walking, Direction::Up, true, {{0, 32, 16, 16, CHICKEN_WALK_MS}, {16, 32, 16, 16, CHICKEN_WALK_MS}, {32, 32, 16, 16, CHICKEN_WALK_MS}, {48, 32, 16, 16, CHICKEN_WALK_MS}}),
    };

    // Junimo
    inline constexpr ClipSheet JUNIMO_SHEET{SpriteId::Junimo};

    constexpr ClipDef<JunimoState> junimo_dance(JunimoState s, int ms) {
        return clip(s, Direction::None, true,
                    {{64, 80, 16, 16, ms}, {0, 16, 16, 16, ms}, {80, 80, 16, 16, ms}, {16, 16, 16, 16, ms},
                     {96, 80, 16, 16, ms}, {32, 16, 16, 16, ms}, {112, 80, 16, 16, ms}, {48, 16, 16, 16, ms}});
    }

    inline constexpr ClipDef<JunimoState> JUNIMO_CLIPS[] = {
        junimo_dance(JunimoState::DanceSlow, 210),
        junimo_dance(JunimoState::Dance,     180),
        junimo_dance(JunimoState::DanceFast, 150),
    };

    // Skeleton
    inline constexpr ClipSheet SKELETON_SHEET{SpriteId::Skeleton, Dimentions::SKELETON_CELL_H - Dimentions::SKELETON_FEET_Y};
    inline constexpr ClipSheet SKELETON_BONES_SHEET{SpriteId::Skeleton};

    inline constexpr int SKELETON_IDLE_MS   = 400;
    inline constexpr int SKELETON_WALK_MS   = 160;
    inline constexpr int SKELETON_ATTACK_MS = 120;

    inline constexpr ClipDef<EnemyState> SKELETON_CLIPS[] = {
        clip(EnemyState::Idle, Direction::Down, true, {{0, 0, 16, 32, SKELETON_IDLE_MS}}),
        clip(EnemyState::Idle, Direction::Right, true, {{0, 32, 16, 32, SKELETON_IDLE_MS}}),
        clip(EnemyState::Idle, Direction::Up, true, {{0, 64, 16, 32, SKELETON_IDLE_MS}}),
        clip(EnemyState::Idle, Direction::Left, true, {{0, 96, 16, 32, SKELETON_IDLE_MS}}),

        clip(EnemyState::Moving, Direction::Down, true, {{0, 0, 16, 32, SKELETON_WALK_MS}, {16, 0, 16, 32, SKELETON_WALK_MS}, {32, 0, 16, 32, SKELETON_WALK_MS}, {48, 0, 16, 32, SKELETON_WALK_MS}}),
        clip(EnemyState::Moving, Direction::Right, true, {{0, 32, 16, 32, SKELETON_WALK_MS}, {16, 32, 16, 32, SKELETON_WALK_MS}, {32, 32, 16, 32, SKELETON_WALK_MS}, {48, 32, 16, 32, SKELETON_WALK_MS}}),
        clip(EnemyState::Moving, Direction::Up, true, {{0, 64, 16, 32, SKELETON_WALK_MS}, {16, 64, 16, 32, SKELETON_WALK_MS}, {32, 64, 16, 32, SKELETON_WALK_MS}, {48, 64, 16, 32, SKELETON_WALK_MS}}),
        clip(EnemyState::Moving, Direction::Left, true, {{0, 96, 16, 32, SKELETON_WALK_MS}, {16, 96, 16, 32, SKELETON_WALK_MS}, {32, 96, 16, 32, SKELETON_WALK_MS}, {48, 96, 16, 32, SKELETON_WALK_MS}}),

        clip(EnemyState::Attacking, Direction::Down, true, {{0, 160, 16, 32, SKELETON_ATTACK_MS}, {16, 160, 16, 32, SKELETON_ATTACK_MS}, {32, 160, 16, 32, SKELETON_ATTACK_MS}, {48, 160, 16, 32, SKELETON_ATTACK_MS}}),
        clip(EnemyState::Attacking, Direction::Right, true, {{0, 32, 16, 32, 100}, {16, 32, 16, 32, 100}, {32, 32, 16, 32, 100}, {48, 32, 16, 32, 100}}),
        clip(EnemyState::Attacking, Direction::Up, true, {{0, 64, 16, 32, 100}, {16, 64, 16, 32, 100}, {32, 64, 16, 32, 100}, {48, 64, 16, 32, 100}}),
        clip(EnemyState::Attacking, Direction::Left, true, {{0, 96, 16, 32, 100}, {16, 96, 16, 32, 100}, {32, 96, 16, 32, 100}, {48, 96, 16, 32, 100}}),
    };

    inline constexpr ClipDef<EnemyState> SKELETON_DEAD_CLIPS[] = {
        clip(EnemyState::Dead, Direction::Down, true, {{0, 128, 16, 16, 400}}),
        clip(EnemyState::Dead, Direction::Right, true, {{0, 128, 16, 16, 400}}),
        clip(EnemyState::Dead, Direction::Up, true, {{0, 128, 16, 16, 400}}),
        clip(EnemyState::Dead, Direction::Left, true, {{0, 128, 16, 16, 400}}),
    };

    // Golem - (Orc)
    inline constexpr ClipSheet GOLEM_SHEET{SpriteId::StoneGolem, Dimentions::GOLEM_CELL_H - Dimentions::GOLEM_FEET_Y};
    inline constexpr ClipSheet GOLEM_ROCKS_SHEET{SpriteId::StoneGolem};

    inline constexpr int GOLEM_IDLE_MS   = 400;
    inline constexpr int GOLEM_WALK_MS   = 200;
    inline constexpr int GOLEM_ATTACK_MS = 130;

    inline constexpr ClipDef<EnemyState> GOLEM_CLIPS[] = {
        clip(EnemyState::Idle, Direction::Down, true, {{0, 0, 16, 24, GOLEM_IDLE_MS}}),
        clip(EnemyState::Idle, Direction::Right, true, {{0, 24, 16, 24, GOLEM_IDLE_MS}}),
        clip(EnemyState::Idle, Direction::Up, true, {{0, 48, 16, 24, GOLEM_IDLE_MS}}),
        clip(EnemyState::Idle, Direction::Left, true, {{0, 72, 16, 24, GOLEM_IDLE_MS}}),

        clip(EnemyState::Moving, Direction::Down, true, {{0, 0, 16, 24, GOLEM_WALK_MS}, {16, 0, 16, 24, GOLEM_WALK_MS}, {32, 0, 16, 24, GOLEM_WALK_MS}, {48, 0, 16, 24, GOLEM_WALK_MS}}),
        clip(EnemyState::Moving, Direction::Right, true, {{0, 24, 16, 24, GOLEM_WALK_MS}, {16, 24, 16, 24, GOLEM_WALK_MS}, {32, 24, 16, 24, GOLEM_WALK_MS}, {48, 24, 16, 24, GOLEM_WALK_MS}}),
        clip(EnemyState::Moving, Direction::Up, true, {{0, 48, 16, 24, GOLEM_WALK_MS}, {16, 48, 16, 24, GOLEM_WALK_MS}, {32, 48, 16, 24, GOLEM_WALK_MS}, {48, 48, 16, 24, GOLEM_WALK_MS}}),
        clip(EnemyState::Moving, Direction::Left, true, {{0, 72, 16, 24, GOLEM_WALK_MS}, {16, 72, 16, 24, GOLEM_WALK_MS}, {32, 72, 16, 24, GOLEM_WALK_MS}, {48, 72, 16, 24, GOLEM_WALK_MS}}),

        clip(EnemyState::Attacking, Direction::Down, true, {{0, 0, 16, 24, GOLEM_ATTACK_MS}, {16, 0, 16, 24, GOLEM_ATTACK_MS}, {32, 0, 16, 24, GOLEM_ATTACK_MS}, {48, 0, 16, 24, GOLEM_ATTACK_MS}}),
        clip(EnemyState::Attacking, Direction::Right, true, {{0, 24, 16, 24, GOLEM_ATTACK_MS}, {16, 24, 16, 24, GOLEM_ATTACK_MS}, {32, 24, 16, 24, GOLEM_ATTACK_MS}, {48, 24, 16, 24, GOLEM_ATTACK_MS}}),
        clip(EnemyState::Attacking, Direction::Up, true, {{0, 48, 16, 24, GOLEM_ATTACK_MS}, {16, 48, 16, 24, GOLEM_ATTACK_MS}, {32, 48, 16, 24, GOLEM_ATTACK_MS}, {48, 48, 16, 24, GOLEM_ATTACK_MS}}),
        clip(EnemyState::Attacking, Direction::Left, true, {{0, 72, 16, 24, GOLEM_ATTACK_MS}, {16, 72, 16, 24, GOLEM_ATTACK_MS}, {32, 72, 16, 24, GOLEM_ATTACK_MS}, {48, 72, 16, 24, GOLEM_ATTACK_MS}}),
    };

    inline constexpr ClipDef<EnemyState> GOLEM_DEAD_CLIPS[] = {
        clip(EnemyState::Dead, Direction::Down, true, {{0, 144, 16, 16, 400}}),
        clip(EnemyState::Dead, Direction::Right, true, {{0, 144, 16, 16, 400}}),
        clip(EnemyState::Dead, Direction::Up, true, {{0, 144, 16, 16, 400}}),
        clip(EnemyState::Dead, Direction::Left, true, {{0, 144, 16, 16, 400}}),
    };

    // Bats
    inline constexpr ClipSheet FROST_BAT_SHEET{SpriteId::FrostBat, Dimentions::BAT_CELL_H - Dimentions::BAT_FEET_Y};
    inline constexpr ClipSheet LAVA_BAT_SHEET{SpriteId::LavaBat, Dimentions::BAT_CELL_H - Dimentions::BAT_FEET_Y};
    inline constexpr ClipSheet FROST_BAT_FRENZY_SHEET{SpriteId::FrostBatFrenzy, Dimentions::BAT_CELL_FRENZY_H - Dimentions::BAT_FRENZY_FEET_Y};
    inline constexpr ClipSheet LAVA_BAT_FRENZY_SHEET{SpriteId::LavaBatFrenzy, Dimentions::BAT_CELL_FRENZY_H - Dimentions::BAT_FRENZY_FEET_Y};
    inline constexpr ClipSheet FROST_BAT_DEBRIS_SHEET{SpriteId::FrostBat};
    inline constexpr ClipSheet LAVA_BAT_DEBRIS_SHEET{SpriteId::LavaBat};

    inline constexpr int BAT_FLAP_MS   = 130;
    inline constexpr int BAT_ATTACK_MS = 90;

    inline constexpr ClipDef<EnemyState> BAT_CLIPS[] = {
        clip(EnemyState::Idle, Direction::Down, true, {{0, 0, 16, 16, BAT_FLAP_MS}, {16, 0, 16, 16, BAT_FLAP_MS}, {32, 0, 16, 16, BAT_FLAP_MS}, {48, 0, 16, 16, BAT_FLAP_MS}}),
        clip(EnemyState::Idle, Direction::Right, true, {{0, 0, 16, 16, BAT_FLAP_MS}, {16, 0, 16, 16, BAT_FLAP_MS}, {32, 0, 16, 16, BAT_FLAP_MS}, {48, 0, 16, 16, BAT_FLAP_MS}}),
        clip(EnemyState::Idle, Direction::Left, true, {{0, 0, 16, 16, BAT_FLAP_MS}, {16, 0, 16, 16, BAT_FLAP_MS}, {32, 0, 16, 16, BAT_FLAP_MS}, {48, 0, 16, 16, BAT_FLAP_MS}}),
        clip(EnemyState::Idle, Direction::Up, true, {{0, 0, 16, 16, BAT_FLAP_MS}, {16, 0, 16, 16, BAT_FLAP_MS}, {32, 0, 16, 16, BAT_FLAP_MS}, {48, 0, 16, 16, BAT_FLAP_MS}}),

        clip(EnemyState::Moving, Direction::Down, true, {{0, 0, 16, 16, BAT_FLAP_MS}, {16, 0, 16, 16, BAT_FLAP_MS}, {32, 0, 16, 16, BAT_FLAP_MS}, {48, 0, 16, 16, BAT_FLAP_MS}}),
        clip(EnemyState::Moving, Direction::Right, true, {{0, 0, 16, 16, BAT_FLAP_MS}, {16, 0, 16, 16, BAT_FLAP_MS}, {32, 0, 16, 16, BAT_FLAP_MS}, {48, 0, 16, 16, BAT_FLAP_MS}}),
        clip(EnemyState::Moving, Direction::Left, true, {{0, 0, 16, 16, BAT_FLAP_MS}, {16, 0, 16, 16, BAT_FLAP_MS}, {32, 0, 16, 16, BAT_FLAP_MS}, {48, 0, 16, 16, BAT_FLAP_MS}}),
        clip(EnemyState::Moving, Direction::Up, true, {{0, 0, 16, 16, BAT_FLAP_MS}, {16, 0, 16, 16, BAT_FLAP_MS}, {32, 0, 16, 16, BAT_FLAP_MS}, {48, 0, 16, 16, BAT_FLAP_MS}}),

        clip(EnemyState::Attacking, Direction::Down, true, {{0, 0, 16, 16, BAT_ATTACK_MS}, {16, 0, 16, 16, BAT_ATTACK_MS}, {32, 0, 16, 16, BAT_ATTACK_MS}, {48, 0, 16, 16, BAT_ATTACK_MS}}),
        clip(EnemyState::Attacking, Direction::Right, true, {{0, 0, 16, 16, BAT_ATTACK_MS}, {16, 0, 16, 16, BAT_ATTACK_MS}, {32, 0, 16, 16, BAT_ATTACK_MS}, {48, 0, 16, 16, BAT_ATTACK_MS}}),
        clip(EnemyState::Attacking, Direction::Left, true, {{0, 0, 16, 16, BAT_ATTACK_MS}, {16, 0, 16, 16, BAT_ATTACK_MS}, {32, 0, 16, 16, BAT_ATTACK_MS}, {48, 0, 16, 16, BAT_ATTACK_MS}}),
        clip(EnemyState::Attacking, Direction::Up, true, {{0, 0, 16, 16, BAT_ATTACK_MS}, {16, 0, 16, 16, BAT_ATTACK_MS}, {32, 0, 16, 16, BAT_ATTACK_MS}, {48, 0, 16, 16, BAT_ATTACK_MS}}),
    };

    inline constexpr ClipDef<EnemyState> BAT_FRENZY_CLIPS[] = {
        clip(EnemyState::Idle, Direction::Down, true, {{0, 0, 16, 17, BAT_FLAP_MS}, {16, 0, 16, 17, BAT_FLAP_MS}, {32, 0, 16, 17, BAT_FLAP_MS}, {48, 0, 16, 17, BAT_FLAP_MS}}),
        clip(EnemyState::Idle, Direction::Right, true, {{0, 0, 16, 17, BAT_FLAP_MS}, {16, 0, 16, 17, BAT_FLAP_MS}, {32, 0, 16, 17, BAT_FLAP_MS}, {48, 0, 16, 17, BAT_FLAP_MS}}),
        clip(EnemyState::Idle, Direction::Left, true, {{0, 0, 16, 17, BAT_FLAP_MS}, {16, 0, 16, 17, BAT_FLAP_MS}, {32, 0, 16, 17, BAT_FLAP_MS}, {48, 0, 16, 17, BAT_FLAP_MS}}),
        clip(EnemyState::Idle, Direction::Up, true, {{0, 0, 16, 17, BAT_FLAP_MS}, {16, 0, 16, 17, BAT_FLAP_MS}, {32, 0, 16, 17, BAT_FLAP_MS}, {48, 0, 16, 17, BAT_FLAP_MS}}),

        clip(EnemyState::Moving, Direction::Down, true, {{0, 0, 16, 17, BAT_FLAP_MS}, {16, 0, 16, 17, BAT_FLAP_MS}, {32, 0, 16, 17, BAT_FLAP_MS}, {48, 0, 16, 17, BAT_FLAP_MS}}),
        clip(EnemyState::Moving, Direction::Right, true, {{0, 0, 16, 17, BAT_FLAP_MS}, {16, 0, 16, 17, BAT_FLAP_MS}, {32, 0, 16, 17, BAT_FLAP_MS}, {48, 0, 16, 17, BAT_FLAP_MS}}),
        clip(EnemyState::Moving, Direction::Left, true, {{0, 0, 16, 17, BAT_FLAP_MS}, {16, 0, 16, 17, BAT_FLAP_MS}, {32, 0, 16, 17, BAT_FLAP_MS}, {48, 0, 16, 17, BAT_FLAP_MS}}),
        clip(EnemyState::Moving, Direction::Up, true, {{0, 0, 16, 17, BAT_FLAP_MS}, {16, 0, 16, 17, BAT_FLAP_MS}, {32, 0, 16, 17, BAT_FLAP_MS}, {48, 0, 16, 17, BAT_FLAP_MS}}),

        clip(EnemyState::Attacking, Direction::Down, true, {{0, 0, 16, 17, BAT_ATTACK_MS}, {16, 0, 16, 17, BAT_ATTACK_MS}, {32, 0, 16, 17, BAT_ATTACK_MS}, {48, 0, 16, 17, BAT_ATTACK_MS}}),
        clip(EnemyState::Attacking, Direction::Right, true, {{0, 0, 16, 17, BAT_ATTACK_MS}, {16, 0, 16, 17, BAT_ATTACK_MS}, {32, 0, 16, 17, BAT_ATTACK_MS}, {48, 0, 16, 17, BAT_ATTACK_MS}}),
        clip(EnemyState::Attacking, Direction::Left, true, {{0, 0, 16, 17, BAT_ATTACK_MS}, {16, 0, 16, 17, BAT_ATTACK_MS}, {32, 0, 16, 17, BAT_ATTACK_MS}, {48, 0, 16, 17, BAT_ATTACK_MS}}),
        clip(EnemyState::Attacking, Direction::Up, true, {{0, 0, 16, 17, BAT_ATTACK_MS}, {16, 0, 16, 17, BAT_ATTACK_MS}, {32, 0, 16, 17, BAT_ATTACK_MS}, {48, 0, 16, 17, BAT_ATTACK_MS}}),
    };

    inline constexpr ClipDef<EnemyState> BAT_DEAD_CLIPS[] = {
        clip(EnemyState::Dead, Direction::Down, true, {{0, 16, 16, 16, 400}}),
        clip(EnemyState::Dead, Direction::Right, true, {{0, 16, 16, 16, 400}}),
        clip(EnemyState::Dead, Direction::Up, true, {{0, 16, 16, 16, 400}}),
        clip(EnemyState::Dead, Direction::Left, true, {{0, 16, 16, 16, 400}}),
    };

    // Skull
    inline constexpr ClipSheet SKULL_SHEET{SpriteId::Skull, -Dimentions::SKULL_HOVER_Y};

    inline constexpr int SKULL_IDLE_MS   = 400;
    inline constexpr int SKULL_MOVE_MS   = 200;
    inline constexpr int SKULL_ATTACK_MS = 120;

    inline constexpr ClipDef<EnemyState> SKULL_CLIPS[] = {
        clip(EnemyState::Idle, Direction::Up, true, {{0, 0, 16, 16, SKULL_IDLE_MS}}),
        clip(EnemyState::Idle, Direction::Right, true, {{0, 16, 16, 16, SKULL_IDLE_MS}}),
        clip(EnemyState::Idle, Direction::Down, true, {{0, 32, 16, 16, SKULL_IDLE_MS}}),
        clip(EnemyState::Idle, Direction::Left, true, {{0, 48, 16, 16, SKULL_IDLE_MS}}),

        clip(EnemyState::Moving, Direction::Up, true, {{0, 0, 16, 16, SKULL_MOVE_MS}}),
        clip(EnemyState::Moving, Direction::Right, true, {{0, 16, 16, 16, SKULL_MOVE_MS}}),
        clip(EnemyState::Moving, Direction::Down, true, {{0, 32, 16, 16, SKULL_MOVE_MS}}),
        clip(EnemyState::Moving, Direction::Left, true, {{0, 48, 16, 16, SKULL_MOVE_MS}}),

        clip(EnemyState::Attacking, Direction::Up, true, {{16, 0, 16, 16, SKULL_ATTACK_MS}}),
        clip(EnemyState::Attacking, Direction::Right, true, {{16, 16, 16, 16, SKULL_ATTACK_MS}}),
        clip(EnemyState::Attacking, Direction::Down, true, {{16, 32, 16, 16, SKULL_ATTACK_MS}}),
        clip(EnemyState::Attacking, Direction::Left, true, {{16, 48, 16, 16, SKULL_ATTACK_MS}}),
    };

    // Spikes
    inline constexpr ClipSheet SPIKE_SHEET{SpriteId::SpikeTrap};

    inline constexpr int SPIKE_STATE_MS = 400;

    inline constexpr ClipDef<TrapState> SPIKE_CLIPS[] = {
        clip(TrapState::Inactive, Direction::None, true, {{0, 0, 16, 16, SPIKE_STATE_MS}}),
        clip(TrapState::Warning, Direction::None, true, {{16, 0, 16, 16, SPIKE_STATE_MS}}),
        clip(TrapState::Active, Direction::None, true, {{32, 0, 16, 16, SPIKE_STATE_MS}}),
    };

    // Wall flame
    inline constexpr ClipSheet WALL_FLAME_SHEET{SpriteId::WallFlameTrap};

    inline constexpr int WALL_FLAME_STATE_MS   = 400;
    inline constexpr int WALL_FLAME_WARNING_MS = 150;

    inline constexpr ClipDef<TrapState> WALL_FLAME_CLIPS[] = {
        clip(TrapState::Inactive, Direction::Down, true,  {{ 0, 22, 11, 15, WALL_FLAME_STATE_MS}}),
        clip(TrapState::Warning,  Direction::Down, false, {{11, 22, 11, 15, WALL_FLAME_WARNING_MS},
                                                           {22, 22, 11, 15, WALL_FLAME_WARNING_MS},
                                                           {33, 22, 11, 15, WALL_FLAME_WARNING_MS}}),
        clip(TrapState::Active,   Direction::Down, true,  {{44, 22, 11, 15, WALL_FLAME_STATE_MS}}),

        clip(TrapState::Inactive, Direction::Up, true,  {{ 0, 37, 11, 19, WALL_FLAME_STATE_MS}}),
        clip(TrapState::Warning,  Direction::Up, false, {{11, 37, 11, 19, WALL_FLAME_WARNING_MS},
                                                         {22, 37, 11, 19, WALL_FLAME_WARNING_MS},
                                                         {33, 37, 11, 19, WALL_FLAME_WARNING_MS}}),
        clip(TrapState::Active,   Direction::Up, true,  {{44, 37, 11, 19, WALL_FLAME_STATE_MS}}),

        clip(TrapState::Inactive, Direction::Right, true,  {{ 0,  0, 15, 11, WALL_FLAME_STATE_MS}}),
        clip(TrapState::Warning,  Direction::Right, false, {{19,  0, 15, 11, WALL_FLAME_WARNING_MS},
                                                            {39,  0, 15, 11, WALL_FLAME_WARNING_MS},
                                                            { 0, 11, 15, 11, WALL_FLAME_WARNING_MS}}),
        clip(TrapState::Active,   Direction::Right, true,  {{19, 11, 15, 11, WALL_FLAME_STATE_MS}}),
    };

    // Chest
    inline constexpr ClipSheet CHEST_SHEET{SpriteId::Craftables};

    inline constexpr int CHEST_FRAME_MS = 60;

    inline constexpr ClipDef<ChestState> CHEST_CLIPS[] = {
        clip(ChestState::Closed,  Direction::None, true,  {{ 32, 512, Dimentions::CHEST_CELL_W, Dimentions::CHEST_CELL_H, CHEST_FRAME_MS}}),
        clip(ChestState::Opening, Direction::None, false, {{ 32, 512, Dimentions::CHEST_CELL_W, Dimentions::CHEST_CELL_H, CHEST_FRAME_MS},
                                                           { 48, 512, Dimentions::CHEST_CELL_W, Dimentions::CHEST_CELL_H, CHEST_FRAME_MS},
                                                           { 64, 512, Dimentions::CHEST_CELL_W, Dimentions::CHEST_CELL_H, CHEST_FRAME_MS},
                                                           { 80, 512, Dimentions::CHEST_CELL_W, Dimentions::CHEST_CELL_H, CHEST_FRAME_MS},
                                                           { 96, 512, Dimentions::CHEST_CELL_W, Dimentions::CHEST_CELL_H, CHEST_FRAME_MS},
                                                           {112, 512, Dimentions::CHEST_CELL_W, Dimentions::CHEST_CELL_H, CHEST_FRAME_MS}}),
        clip(ChestState::Closing, Direction::None, false, {{112, 512, Dimentions::CHEST_CELL_W, Dimentions::CHEST_CELL_H, CHEST_FRAME_MS},
                                                           { 96, 512, Dimentions::CHEST_CELL_W, Dimentions::CHEST_CELL_H, CHEST_FRAME_MS},
                                                           { 80, 512, Dimentions::CHEST_CELL_W, Dimentions::CHEST_CELL_H, CHEST_FRAME_MS},
                                                           { 64, 512, Dimentions::CHEST_CELL_W, Dimentions::CHEST_CELL_H, CHEST_FRAME_MS},
                                                           { 48, 512, Dimentions::CHEST_CELL_W, Dimentions::CHEST_CELL_H, CHEST_FRAME_MS},
                                                           { 32, 512, Dimentions::CHEST_CELL_W, Dimentions::CHEST_CELL_H, CHEST_FRAME_MS}}),
    };

    // Flames
    inline constexpr ClipSheet FLAME_SHEET{SpriteId::Flames, Dimentions::FLAME_CELL_H / 2};

    inline constexpr int FLAME_FRAME_MS = 80;

    inline constexpr ClipDef<FlameType> FLAME_CLIPS[] = {
        clip(FlameType::Green, Direction::None, true, {{0, 0, 8, 8, FLAME_FRAME_MS}, {8, 0, 8, 8, FLAME_FRAME_MS}, {16, 0, 8, 8, FLAME_FRAME_MS}, {24, 0, 8, 8, FLAME_FRAME_MS}, {32, 0, 8, 8, FLAME_FRAME_MS}, {40, 0, 8, 8, FLAME_FRAME_MS}, {48, 0, 8, 8, FLAME_FRAME_MS}}),
        clip(FlameType::Blue, Direction::None, true, {{0, 8, 8, 8, FLAME_FRAME_MS}, {8, 8, 8, 8, FLAME_FRAME_MS}, {16, 8, 8, 8, FLAME_FRAME_MS}, {24, 8, 8, 8, FLAME_FRAME_MS}, {32, 8, 8, 8, FLAME_FRAME_MS}, {40, 8, 8, 8, FLAME_FRAME_MS}, {48, 8, 8, 8, FLAME_FRAME_MS}}),
        clip(FlameType::Common, Direction::None, true, {{0, 16, 8, 8, FLAME_FRAME_MS}, {8, 16, 8, 8, FLAME_FRAME_MS}, {16, 16, 8, 8, FLAME_FRAME_MS}, {24, 16, 8, 8, FLAME_FRAME_MS}, {32, 16, 8, 8, FLAME_FRAME_MS}, {40, 16, 8, 8, FLAME_FRAME_MS}, {48, 16, 8, 8, FLAME_FRAME_MS}}),
    };

    inline const std::map<AnimalType, SpriteId> ANIMAL_SPRITE_IDS = {
        {AnimalType::BrownCow,     SpriteId::BronwCow},
        {AnimalType::WhiteCow,     SpriteId::WhiteCow},
        {AnimalType::BrownChicken, SpriteId::BronwChicken},
        {AnimalType::WhiteChicken, SpriteId::WhiteChicken},
        {AnimalType::BlueChicken,  SpriteId::BlueChicken},
        {AnimalType::VoidChicken,  SpriteId::VoidChicken},
    };

    struct EnemyVisual { SpriteId sprite; SpriteId frenzy_sprite; int cell_h; };
    inline const std::map<EnemyType, EnemyVisual> ENEMY_VISUALS = {
        {EnemyType::Skeleton,      {SpriteId::Skeleton,   SpriteId::Skeleton,       Dimentions::SKELETON_CELL_H}},
        {EnemyType::Orc,           {SpriteId::StoneGolem, SpriteId::StoneGolem,     Dimentions::GOLEM_CELL_H}},
        {EnemyType::VampireMale,   {SpriteId::FrostBat,   SpriteId::FrostBatFrenzy, Dimentions::BAT_CELL_H}},
        {EnemyType::VampireFemale, {SpriteId::LavaBat,    SpriteId::LavaBatFrenzy,  Dimentions::BAT_CELL_H}},
        {EnemyType::Skull,         {SpriteId::Skull,      SpriteId::Skull,          Dimentions::SKULL_CELL_H}},
    };

    struct TrapVisual { SpriteId sprite; int cell_h; int depth_offset; bool directional; };
    inline const std::map<TrapType, TrapVisual> TRAP_VISUALS = {
        {TrapType::Spikes,    {SpriteId::SpikeTrap,     Dimentions::SPIKE_CELL_H,     -TILE_TO_PIXEL, false}},
        {TrapType::WallFlame, {SpriteId::WallFlameTrap, Dimentions::WALL_FLAME_MAX_H, 0,              true}},
    };

    inline const std::map<TileTextureId, std::vector<Sprite>> TILE_PIECES = [] {
        std::map<TileTextureId, std::vector<Sprite>> m;
        for (const auto& [id, tile] : TILES) {
            const SDL2pp::Rect r = tile.src();
            const int cols = std::max(1, r.w / TILE_TO_PIXEL), rows = std::max(1, r.h / TILE_TO_PIXEL);
            for (int row = 0; row < rows; ++row)
                for (int col = 0; col < cols; ++col)
                    m[id].push_back(Sprite::from(tile.get_sprite_id(), SDL2pp::Rect(r.x + col * TILE_TO_PIXEL, r.y + row * TILE_TO_PIXEL,
                                                 std::min(TILE_TO_PIXEL, r.w), std::min(TILE_TO_PIXEL, r.h))));
        }
        return m;
    }();

    inline const std::map<std::pair<ItemId, Direction>, std::vector<ToolFrame>> TOOL_FRAMES = [] {
        std::map<std::pair<ItemId, Direction>, std::vector<ToolFrame>> m;
        for (const auto& d : TOOL_ANIMS) {
            auto& frames = m[{d.item, d.dir}];
            for (const auto& s : d.steps) {
                const ToolRect& r = d.sprites[s.sprite];
                frames.emplace_back(Sprite(d.sheet, r.x, r.y, r.w, r.h,
                                           static_cast<int16_t>(s.off.x - Dimentions::FARMER_CELL_W / 2),
                                           static_cast<int16_t>(s.off.y - Dimentions::FARMER_FEET_Y)),
                                    s.angle, SDL2pp::Point(d.pivot_x, d.pivot_y));
            }
        }
        return m;
    }();

    template <class State, std::size_t N>
    void add_clips(std::map<ClipId, Clip>& m, const ClipSheet& sheet, const ClipDef<State> (&defs)[N], bool mirror,
                   LayerOffsets (*offsets_of)(uint16_t, uint16_t) = nullptr) {
        auto build = [&](const ClipDef<State>& d, bool flip) {
            Clip c{{}, d.loop};
            for (int i = 0; i < d.count; ++i) {
                const FrameRect& f = d.frames[i];
                const Sprite s(sheet.sprite, f.x, f.y, f.w, f.h, static_cast<int16_t>(-f.w / 2), static_cast<int16_t>(-(f.h - sheet.baseline)));
                c.sequence.push_back({AnimatedFrame{s, flip, offsets_of ? offsets_of(f.x, f.y) : LayerOffsets{}}, f.ms});
            }
            return c;
        };
        for (const auto& d : defs) {
            m.insert_or_assign(ClipId{sheet.sprite, Action{d.state}, d.dir, d.item}, build(d, false));
            if (mirror && d.dir == Direction::Right)
                m.insert_or_assign(ClipId{sheet.sprite, Action{d.state}, Direction::Left, d.item}, build(d, true));
        }
    }

    inline const std::map<ClipId, Clip> CLIPS = [] {
        std::map<ClipId, Clip> m;
        add_clips(m, FARMER_SHEET, FARMER_CLIPS, FARMER_MIRROR_LEFT, farmer_offsets);
        for (SpriteId id : {SpriteId::BronwCow, SpriteId::WhiteCow})
            add_clips(m, {id, COW_SHEET.baseline}, COW_CLIPS, true);
        for (SpriteId id : {SpriteId::BronwChicken, SpriteId::WhiteChicken, SpriteId::BlueChicken, SpriteId::VoidChicken})
            add_clips(m, {id, CHICKEN_SHEET.baseline}, CHICKEN_CLIPS, true);
        add_clips(m, CHEST_SHEET, CHEST_CLIPS, false);
        add_clips(m, SKELETON_SHEET, SKELETON_CLIPS, false);
        add_clips(m, SKELETON_BONES_SHEET, SKELETON_DEAD_CLIPS, false);
        add_clips(m, GOLEM_SHEET, GOLEM_CLIPS, false);
        add_clips(m, GOLEM_ROCKS_SHEET, GOLEM_DEAD_CLIPS, false);
        add_clips(m, FROST_BAT_SHEET, BAT_CLIPS, false);
        add_clips(m, FROST_BAT_DEBRIS_SHEET, BAT_DEAD_CLIPS, false);
        add_clips(m, FROST_BAT_FRENZY_SHEET, BAT_FRENZY_CLIPS, false);
        add_clips(m, LAVA_BAT_SHEET, BAT_CLIPS, false);
        add_clips(m, LAVA_BAT_DEBRIS_SHEET, BAT_DEAD_CLIPS, false);
        add_clips(m, LAVA_BAT_FRENZY_SHEET, BAT_FRENZY_CLIPS, false);
        add_clips(m, SKULL_SHEET, SKULL_CLIPS, false);
        add_clips(m, JUNIMO_SHEET, JUNIMO_CLIPS, false);
        add_clips(m, SPIKE_SHEET, SPIKE_CLIPS, false);
        add_clips(m, WALL_FLAME_SHEET, WALL_FLAME_CLIPS, true);
        add_clips(m, FLAME_SHEET, FLAME_CLIPS, false);
        return m;
    }();
}