#pragma once

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>
#include <optional>
#include <string>
#include "protocol_types.h"
#include "dtos.h"
#include "constants.h"
#include "../animator.h"
#include "../utils.h"
#include "../render_info.h"
#include "tool_view.h"

namespace farmer {

// const de sprite info del farmer
constexpr int CELL_W = 16;
constexpr int CELL_H = 32;
constexpr int COLS = 6;

constexpr int BODY_REGION_W = 96;
constexpr int TILE_W = 288;
constexpr int TILE_H = 672;

constexpr int FEET_Y = 30;

constexpr int HAIR_PER_ROW = 16;
constexpr int HAIR_BLOCK_H = 96;
constexpr int HAIR_ABOVE_HEAD = 4;
constexpr int SHIRT_SIZE = 8;
constexpr int SHIRTS_PER_ROW = 16;
constexpr int SHIRT_BLOCK_H = 32;
constexpr int SHIRT_X = 4;
constexpr int SHIRT_BELOW_HEAD = 10;

constexpr int PANTS_BLOCK_W = 192;
constexpr int PANTS_BLOCK_H = 688;
constexpr int PANTS_PER_ROW = 10;

constexpr int TOOL_SIZE = 16;

}


class PlayerView {
private:
    Position pos{0, 0};
    Direction facing = Direction::Down;
    PlayerState state = PlayerState::Idle;
    std::string name;

    // appearence
    uint16_t shirt_model = 0;
    uint16_t pant_model = 0;
    uint16_t hair_model = 0;
    SDL_Color pant_color{255, 255, 255, 255};
    SDL_Color hair_color{255, 255, 255, 255};

    // TBD: std::optional<ToolView> currentTool;

    Animator anim;
    std::optional<ClipId> current_clip;
    long long last_it = -1;

    struct Cell {
        SDL2pp::Point origin;
        Frame frame;
    };

    void draw_body(const RenderInfo& ctx, const Cell& cell);
    void draw_pants(const RenderInfo& ctx, const Cell& cell);
    void draw_shirt(const RenderInfo& ctx, const Cell& cell);
    void draw_hair(const RenderInfo& ctx, const Cell& cell);
    void draw_arms(const RenderInfo& ctx, const Cell& cell);

public:
    PlayerView(std::string name, const PlayerData& d);

    void update(const PlayerData& d);

    const std::string& get_name() const { return name; }

    // filtro de lado cliente, null si no esta en la vista o su profundidad de capa si esta
    std::optional<float> depth(const RenderInfo& ctx) const;

    // se dibuja solo
    void draw(const RenderInfo& ctx);
};