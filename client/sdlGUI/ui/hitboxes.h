#pragma once
#include <cstddef>
#include <optional>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "../render_info.h"

class Hitboxes {
private:
    std::vector<SDL2pp::Rect> rects;
    float scale_x = 1.0f;
    float scale_y = 1.0f;
    SDL2pp::Point mouse{-1, -1};

    SDL2pp::Point to_output(int x, int y) const {
        return SDL2pp::Point(static_cast<int>(static_cast<float>(x) * scale_x), static_cast<int>(static_cast<float>(y) * scale_y));
    }

public:
    void begin(const RenderInfo& ctx) {
        int window_w = 0, window_h = 0;
        if (SDL_Window* window = SDL_RenderGetWindow(ctx.r.Get())) {
            SDL_GetWindowSize(window, &window_w, &window_h);
        }
        const SDL2pp::Point out = ctx.r.GetOutputSize();
        scale_x = window_w > 0 ? static_cast<float>(out.x) / static_cast<float>(window_w) : 1.0f;
        scale_y = window_h > 0 ? static_cast<float>(out.y) / static_cast<float>(window_h) : 1.0f;

        int mx = 0, my = 0;
        SDL_GetMouseState(&mx, &my);
        mouse = to_output(mx, my);
        rects.clear();
    }

    bool add(const SDL2pp::Rect& r) {
        rects.push_back(r);
        return r.Contains(mouse);
    }

    std::size_t size() const { return rects.size(); }

    std::optional<std::size_t> hit(int x, int y) const {
        const SDL2pp::Point p = to_output(x, y);
        for (std::size_t i = 0; i < rects.size(); ++i) {
            if (rects[i].Contains(p)) return i;
        }
        return std::nullopt;
    }
};
