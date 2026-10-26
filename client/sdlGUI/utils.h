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

enum class SpriteId : uint16_t{

};

enum class SoundId : uint16_t{

};

using Action = std::variant<PlayerState, AnimalState, EnemyState>;

struct ClipId{
    SpriteId sprite;
    Action currentAction;
    Direction dir;
};