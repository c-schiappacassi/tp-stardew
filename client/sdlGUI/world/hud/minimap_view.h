#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include <SDL2pp/SDL2pp.hh>

#include "../../../state.h"
#include "ui_box.h"
#include "../player/player_view.h"

namespace {
    enum class Area : std::size_t { Farm, Town, Mines, Count };

    struct Slot {
        double x, y;
    };

    static constexpr std::size_t MAX_PLAYERS = 4;
    static constexpr double HEAD_SCALE = 0.3;
    static constexpr double SCREEN_FILL = 0.8;
    static constexpr SDL_Color DIM{0, 0, 0, 140};

    static constexpr Slot LAYOUTS[MAX_PLAYERS][MAX_PLAYERS] = {
        {{0.50, 0.50}},
        {{0.35, 0.50}, {0.65, 0.50}},
        {{0.35, 0.35}, {0.65, 0.35}, {0.50, 0.65}},
        {{0.35, 0.35}, {0.65, 0.35}, {0.35, 0.65}, {0.65, 0.65}},
    };

    // TODO: estos son del mapa del juego, hay que cambiarlos a lo que defina Lu en el editor
    inline static const std::vector<SDL2pp::Rect> REGIONS{{
        SDL2pp::Rect(282, 226, 205, 145),
        SDL2pp::Rect(569, 289, 340, 174),
        SDL2pp::Rect(728, 30, 164, 80),
    }};
}

class MinimapView {
private:
    struct Marker {
        uint16_t hair_model;
        SDL_Color hair_color;
    };
    std::array<std::vector<Marker>, static_cast<std::size_t>(Area::Count)> markers;

    static std::optional<Area> area_of(const PlayerData& p);
    static SDL2pp::Rect to_screen(const SDL2pp::Rect& r, const SDL2pp::Rect& map, const SDL2pp::Point& tex);
    static void draw_head(const RenderInfo& ctx, const SDL2pp::Point& center, int size, const Marker& m, const Sprite& head);

public:
    void update(const std::map<std::string, PlayerData>& players);
    void draw(const RenderInfo& ctx) const ;
};