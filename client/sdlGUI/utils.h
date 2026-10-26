#pragma once
#include <SDL2pp/SDL2pp.hh>
#include <string>

struct Sprite {
    SDL2pp::Texture texture;
};

struct Sound {
};

struct Clip {
    int row;
    int frames;
    int fps;
    bool loop;
    bool flip_h;
};

struct Frame {
    int row;
    int col;
    bool flip_h;
};

enum class SpriteId : uint16_t {
    FarmerBase = 1,
    Hairstyles = 2,
    Pants = 3,
    Shirts = 4,
    Tools = 5,
    Weapons = 6
};

enum class SoundId : uint16_t{

};

using Action = std::variant<PlayerState, AnimalState, EnemyState>;

struct ClipId{
    SpriteId sprite;
    Action currentAction;
    Direction dir;
};