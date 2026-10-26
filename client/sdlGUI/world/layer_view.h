#pragma once
#include <SDL2pp/SDL2pp.hh>
#include "../utils.h"
#include "../animator.h"
#include "../render_info.h"

/*
TODO: 
RECORDAR
existen distintos tipos de pelo, que se deben pintar
existen distintas remeras que NO  se pintan
existen distintos tipos de pantalones, que se pintan
*/

class LayerView{
private:
    uint16_t model; // que remera/pantalon/pelo
    Animator anim;
    SpriteId spriteId;
    std::optional<SDL_Color> color;
public:
    void draw(const RenderInfo& ctx);
};