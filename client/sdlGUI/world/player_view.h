#pragma once

#include <SDL2/SDL.h>
#include <array>
#include <optional>
#include <string>
#include <map>
#include "protocol_types.h"
#include "../animator.h"
#include "../utils.h"
#include "../render_info.h"
#include "layer_view.h"
#include "tool_view.h"

class PlayerView {
private:
    std::vector<LayerView> appearence;
    ToolView currentTool;
    Animator anim;
    SpriteId spriteId;

    void draw_body(const RenderInfo& ctx);
    void draw_arms(const RenderInfo& ctx);
public:
    explicit PlayerView(const PlayerData& d);

    // filtro de lado cliente, null si no esta en la vista o su profundidad de capa si esta
    std::optional<float> depth(const RenderInfo& ctx) const;

    // se dibuja solo
    void draw(const RenderInfo& ctx);
};