#pragma once

#include <cstdint>
#include <map>
#include <optional>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "../animator.h"
#include "entity_view.h"
#include "../draw_region.h"
#include "../render_info.h"
#include "../utils.h"

#include "constants.h"
#include "protocol_types.h"

namespace farmer {
constexpr int CELL_W = 16;
constexpr int CELL_H = 32;

constexpr int COLS = 6;
constexpr int BODY_REGION_W = 96;
constexpr int FEET_Y = 30;

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

constexpr int WALK_BOB = 1;

}


class PlayerView : public EntityView {
private:
    Position pos{0, 0};
    Direction facing = Direction::Down;
    PlayerState state = PlayerState::Idle;
    std::optional<ItemId> held_item;

    uint16_t shirt_model = 0;
    uint16_t pant_model = 0;
    uint16_t hair_model = 0;
    SDL_Color pant_color{255, 255, 255, 255};
    SDL_Color hair_color{255, 255, 255, 255};

    Animator anim;
    std::optional<ClipId> current_clip;
    long long last_it = -1;

public:
    explicit PlayerView(const PlayerData& data);

    void update(const PlayerData& data);

    // Devuelve nullopt si el jugador está fuera de la cámara.
    std::optional<float> depth(const RenderInfo& ctx) const override;

    void draw(const RenderInfo& ctx) override;
};