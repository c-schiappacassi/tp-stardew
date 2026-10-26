#pragma once
#include <SDL2pp/SDL2pp.hh>
#include "../utils.h"
#include "../animator.h"
#include "../render_info.h"

/*
TODO: 
LAS ARMAS ESTAN SEPARADAS DEL RESTO DE LAS HERRAMIENTAS, UNIR LOS PNG PARA SIMPLIFICARME LA VIDA
*/

class ToolView{
private:
    uint16_t model; // que herramienta(pala, hacha, etc)
    Animator anim;
    SpriteId spriteId;
    
public:
    void draw(const RenderInfo& ctx);
};