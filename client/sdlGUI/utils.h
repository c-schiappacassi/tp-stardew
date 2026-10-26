#pragma once
#include <SDL2pp/SDL2pp.hh>
#include <string>

struct Sprite {
    SDL2pp::Texture texture;
    // TBD si sirve de algo 
    uint8_t heigth;
    uint8_t width;
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
};

enum class SoundId : uint16_t{

};

using Action = std::variant<PlayerState, AnimalState, EnemyState>;

struct ClipId{
    SpriteId sprite;
    Action currentAction;
    Direction dir;
    std::optional<uint16_t> model; // para el modelo de la ropa y para la herramienta
};