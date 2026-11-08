#pragma once
#include <cstdint>
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
    FarmerBase,
    Hairstyles,
    Pants,
    Shirts,
    Tools,
    Clock,
    Weapons,
    InGameUI,
    Ores,
    MenuTiles,
    Crops,
    BronwCow,
    BronwChicken
};

enum class UiSpriteId : uint8_t {
    HotbarBar,
    HotbarSlot,
    HotbarSlotOutline,
    HotbarSlotSelected
};

struct Drawable {
    float depth;
    uint64_t tiebreak;
    EntityView* view;
};

struct Sprite {
    SpriteId sprite_id;
    uint16_t x, y;
    uint16_t width, height;
    int16_t offset_x = 0;
    int16_t offset_y = 0;

    SDL2pp::Rect src() const { return SDL2pp::Rect(x, y, width, height); }
    int dx(bool flip) const { return flip ? -(offset_x + static_cast<int>(width)) : offset_x; }
};

struct Offset { int8_t x = 0; int8_t y = 0; }; 
struct LayerOffsets { Offset hair, shirt, pants; };

struct ToolFrame {
    Sprite sprite;
    double angle;
    SDL2pp::Point pivot;
};

struct AnimatedFrame {
    Sprite sprite;
    bool flip;
    LayerOffsets offsets{};
};

struct ClipFrame {
    AnimatedFrame frame;
    int duration_ms;
};

struct Clip {
    std::vector<ClipFrame> sequence;
    bool loop = false;
};


using Action = std::variant<PlayerState, AnimalState, EnemyState>;

struct ClipId {
    SpriteId sprite_id;
    Action action;
    Direction direction;
    std::optional<ItemId> item_id = std::nullopt;

    bool operator<(const ClipId& other) const {
        return std::tie(sprite_id, action, direction, item_id) <
               std::tie(other.sprite_id, other.action, other.direction, other.item_id);
    }

    bool operator==(const ClipId& other) const {
        return sprite_id == other.sprite_id &&
               action == other.action &&
               direction == other.direction &&
               item_id == other.item_id;
    }
};

struct Sound{

};

enum class SoundId : uint16_t {

};