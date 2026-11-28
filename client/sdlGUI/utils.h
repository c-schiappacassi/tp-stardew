#pragma once
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>
#include <tuple>

#include <SDL2pp/SDL2pp.hh>

#include "world/entity_view.h"

#include "protocol_types.h"

#define TILE_TO_PIXEL 16

enum class SpriteId : uint16_t {
    PlaceHolder,
    
    FarmerBase,
    Hairstyles,
    Pants,
    Shirts,
    Tools,
    Weapons,
    Ores,
    Crops,

    MenuTiles,
    InGameUI,
    Clock,
    Minimap,
    LightGlow,
    
    BronwCow,
    WhiteCow,
    BronwChicken,
    WhiteChicken,
    BlueChicken,
    VoidChicken,
    
    OutdoorsSpring,
    TilledDirt,
    Trees,
    MarketBuildings,
    FarmBuildings,
    Cabins,
    Mushrooms,
    GrassDecor,
    Bushes,
    TownSpring,
    CavernPlants,
    DungeonWallTiles,
    MinesTiles,
    LevelPortals,
    Furniture,
    
    Skeleton,
    StoneGolem,
    FrostBat,
    LavaBat,
    FrostBatFrenzy,
    LavaBatFrenzy,
    Skull,

    SpikeTrap,
    WallFlameTrap,
    Flames,

    Cursors,
    Emotes,
    ChatBox,
    InfoSing,

    PlayerHouseInterior,
    MarniesInterior,
    PierresInterior,
    ClintsInterior,
    RobinsInterior,
    ComunityCenterInterior,

    Marnie,
    Pierre,
    Clint,
    Robin,
    
    Harvest,
    Craftables,
    NightSky,
    DaySky,
    Junimo,
    TownInteriors
};

enum class UiSpriteId : uint8_t {
    HotbarBar,
    HotbarSlot,
    HotbarSlotOutline,
    HotbarSlotSelected,
    EnergyBar,
    HealthBar,
    ChatBox,
    ChatTypeBox,
    DialogBox,
    OkButton,
    CancelButton,
    PlayerHead,
    InfoSing,
    InfoSingArrow,
    NightSky
};

struct Drawable {
    float depth;
    uint64_t tiebreak;
    EntityView* view;
};

struct Offset { int8_t x = 0; int8_t y = 0; }; 
struct LayerOffsets { Offset hair, shirt, pants; };

enum class ChestState : uint8_t { Closed, Opening, Closing };

enum class JunimoState : uint8_t { DanceSlow, Dance, DanceFast };

using Action = std::variant<PlayerState, AnimalState, EnemyState, TrapState, FlameType, ChestState, JunimoState>;

struct ClipId {
    SpriteId sprite_id;
    Action action;
    Direction direction;
    std::optional<ItemId> item_id = std::nullopt;
};

inline bool operator<(const ClipId& a, const ClipId& b) {
    return std::tie(a.sprite_id, a.action, a.direction, a.item_id) <
           std::tie(b.sprite_id, b.action, b.direction, b.item_id);
}

inline bool operator==(const ClipId& a, const ClipId& b) {
    return a.sprite_id == b.sprite_id && a.action == b.action &&
           a.direction == b.direction && a.item_id == b.item_id;
}

struct Sound {
    std::shared_ptr<Mix_Chunk> chunk;
    float gain = 1.0f;
};
 
enum class SoundId : uint16_t {
    StepGrass,
    StepStone,
    StepWood,
    PlayerHurt,
    SwordSwing,
    PlantWatered,
    SoilTilled,
    EnemyHurtSkeleton,
    EnemyHurtGolem,
    EnemyHurtVampire,
    EnemyHurtGeneric,
    ChickenCall,
    CowMoo,
    AmbienceFarm,
    AmbienceTown,
    AmbienceDungeon,
    OreHit,
    OreBreak,
    ChestOpen,
    VampireFrenzy,
    TrapSpikes,
    TrapFlame,
    SkullFire,
    ChestClose,
};