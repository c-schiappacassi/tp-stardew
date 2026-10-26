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



class PlayerView {
private:
    std::vector<LayerView> appearence; // TODO
    // TODO: ToolView tool
    Animator anim;
public:
    explicit PlayerView(const PlayerData& d);

    // filtro de lado cliente, null si no esta en la vista o su profundidad de capa si esta
    std::optional<float> depth(const RenderInfo& ctx) const;

    // se dibuja solo
    void draw(const RenderInfo& ctx);
};