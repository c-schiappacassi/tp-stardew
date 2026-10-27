#pragma once
#include <SDL2pp/SDL2pp.hh>
#include <compare>
#include <cstdint>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "protocol_types.h"

#define TILE_TO_PIXEL 16

struct Sprite {
    SDL2pp::Texture texture;
    // TBD si poner unidad en pixels o en tiles
    uint16_t width;
    uint16_t heigth;
};

struct Sound {
};


struct Frame {
    int row;
    int col;
    bool flip_h;
};

struct ClipFrame {
    Frame frame;
    int duration_ms;
};

struct Clip {
    std::vector<ClipFrame> sequence;
    bool loop = false;
};

enum class SpriteId : uint16_t {
    FarmerBase = 1,
    Hairstyles = 2,
    Pants = 3,
    Shirts = 4,
    Tools = 5,
};

enum class SoundId : uint16_t{

};

using Action = std::variant<PlayerState, AnimalState, EnemyState>;

struct ClipId {
    SpriteId sprite;
    Action currentAction;
    Direction dir;
    std::optional<uint16_t> model;

    auto operator<=>(const ClipId&) const = default;
    bool operator!=(const ClipId&) const = default;
};