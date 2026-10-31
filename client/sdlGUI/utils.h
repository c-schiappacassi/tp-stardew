#pragma once
#include <compare>
#include <cstdint>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include <SDL2pp/SDL2pp.hh>

#include "world/entity_view.h"

#include "protocol_types.h"

#define TILE_TO_PIXEL 16

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

struct ClipId {
    SpriteId sprite_id;
    Action action;
    Direction direction;

    auto operator<=>(const ClipId&) const = default;
};

using Action = std::variant<PlayerState, AnimalState, EnemyState>;

enum class SpriteId : uint16_t {
    FarmerBase,
    Hairstyles,
    Pants,
    Shirts,
    Tools,
    Clock,
    InGameUI,
    Ores
};

struct Sound{

};

enum class SoundId : uint16_t {

};