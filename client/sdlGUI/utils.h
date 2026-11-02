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
    InGameUI,
    Ores,
    MenuTiles,
};


struct Drawable {
    float depth;
    uint64_t tiebreak;
    EntityView* view;
};

struct Sprite {
    SpriteId sprite_id;
    uint16_t width;
    uint16_t height;
    uint16_t row;
    uint16_t col;
};

struct AnimatedFrame {
    Sprite sprite;
    bool flip_h = false;
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

    bool operator<(const ClipId& other) const {
        return std::tie(sprite_id, action, direction) <
               std::tie(other.sprite_id, other.action, other.direction);
    }

    bool operator==(const ClipId& other) const {
        return sprite_id == other.sprite_id &&
               action == other.action &&
               direction == other.direction;
    }
};

struct Sound{

};

enum class SoundId : uint16_t {

};