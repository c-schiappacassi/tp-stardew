#pragma once

#include <cstdint>
#include <map>
#include <optional>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "../animator.h"
#include "entity_view.h"
#include "lighting.h"
#include "../draw_region.h"
#include "../render_info.h"
#include "../utils.h"

#include "constants.h"
#include "protocol_types.h"

namespace farmer {
constexpr int CELL_W = 16;
constexpr int CELL_H = 32;

constexpr int BODY_REGION_W = 96;

constexpr int HAIR_PER_ROW = 16;
constexpr int HAIR_BLOCK_H = 96;
constexpr Offset HAIR_BASE{0, 1};

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

    uint16_t shirt_model = 0;
    uint16_t pant_model = 0;
    uint16_t hair_model = 0;
    SDL_Color pant_color{255, 255, 255, 255};
    SDL_Color hair_color{255, 255, 255, 255};

    Animator anim;
    std::optional<ClipId> current_clip;
    long long last_it = -1;

    void tick(const RenderInfo& ctx, const ClipId& clip_id);
    void draw_layers_in_order(const RenderInfo& ctx, const farmer::DrawPlayerData& d);
public:
    explicit PlayerView(const PlayerData& data);

    void update(const PlayerData& data);

    std::optional<float> depth(const RenderInfo& ctx) const override;

    void draw(const RenderInfo& ctx) override;

    std::optional<lighting::Light> light(const RenderInfo& ctx) const;
};