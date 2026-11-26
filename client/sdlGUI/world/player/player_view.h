#pragma once

#include <cstdint>
#include <map>
#include <optional>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "../../clip_player.h"
#include "../entity_view.h"
#include "../lighting.h"
#include "../../draw_region.h"
#include "../../render_info.h"
#include "../../utils.h"
#include "../effects/visual/hit_flash.h"
#include "../effects/sounds/footstep_sound.h"
#include "../effects/sounds/player_hurt_sound.h"
#include "../effects/sounds/sword_swing_sound.h"
#include "../effects/triggers/health_trigger.h"
#include "../effects/triggers/step_trigger.h"
#include "../effects/triggers/tool_trigger.h"

#include "constants.h"
#include "protocol_types.h"

namespace farmer {
    constexpr int NAME_SCALE_UP = 2 ;
    constexpr int NAME_OFFSET = 30;
    constexpr SDL_Color NAME_COLOR{255, 255, 255, 255};
    constexpr SDL_Color NAME_SHADOW{40, 25, 10, 255};

    constexpr int CELL_W = 16;
    constexpr int CELL_H = 32;

    constexpr int BODY_REGION_W = 96;

    constexpr int HAIR_PER_ROW = 16;
    constexpr int HAIR_BLOCK_H = 96;
    constexpr Offset HAIR_BASE{0, 1};

    inline SDL2pp::Rect hair_src(uint16_t model, int row = 0, int height = CELL_H) {
        return SDL2pp::Rect((model % HAIR_PER_ROW) * CELL_W, (model / HAIR_PER_ROW) * HAIR_BLOCK_H + row * CELL_H, CELL_W, height);
    }

    constexpr int SHIRT_SIZE = 8;
    constexpr int SHIRTS_PER_ROW = 16;
    constexpr int SHIRT_BLOCK_H = 32;
    constexpr Offset SHIRT_BASE{4, 15};


    constexpr int PANTS_BLOCK_W = 192;
    constexpr int PANTS_BLOCK_H = 688;
    constexpr int PANTS_PER_ROW = 10;

    struct DrawPlayerData {
        SDL2pp::Point anchor;
        SDL2pp::Point origin;
        SDL2pp::Rect body_src;
        SDL2pp::Rect arms;
        LayerOffsets offsets;
        size_t frame_index;
        int cell_w;
        bool flip;
    };
}


class PlayerView : public EntityView {
private:
    Position pos{0, 0};
    Direction facing = Direction::Down;
    PlayerState state = PlayerState::Idle;
    std::optional<ItemId> held_item;
    std::optional<ItemId> current_tool;
    std::string name;
    SoundEmitter sfx;
    HealthTrigger health_trigger;
    HitFlash* hit_flash = nullptr;
    StepTrigger step_trigger;
    ToolTrigger tool_trigger;
    
    uint16_t shirt_model = 0;
    uint16_t pant_model = 0;
    uint16_t hair_model = 0;
    SDL_Color pant_color{255, 255, 255, 255};
    SDL_Color hair_color{255, 255, 255, 255};
    
    ClipPlayer clip_player;

    void draw_layers_in_order(const RenderInfo& ctx, const farmer::DrawPlayerData& d, const std::optional<SDL_Color>& flash);
public:
    explicit PlayerView(const PlayerData& data);

    void update(const PlayerData& data);

    std::optional<float> depth(const RenderInfo& ctx) const override;

    void draw(const RenderInfo& ctx) override;
    void draw_name(const RenderInfo& ctx) const;

    std::optional<lighting::Light> light(const RenderInfo& ctx) const;
};