#pragma once
#include <cstddef>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "../../render_info.h"
#include "../ui_layer.h"

class ModalLayer : public UiLayer {
private:
    std::vector<SDL2pp::Rect> buttons;
    float scale_x = 1.0f;
    float scale_y = 1.0f;

    static bool inside(const SDL2pp::Rect& r, int x, int y) {
        return x >= r.x && x < r.x + r.w && y >= r.y && y < r.y + r.h;
    }

protected:
    void sync_hitboxes(const RenderInfo& ctx, std::vector<SDL2pp::Rect> new_buttons) {
        int window_w = 0, window_h = 0;
        if (SDL_Window* window = SDL_RenderGetWindow(ctx.r.Get())) {
            SDL_GetWindowSize(window, &window_w, &window_h);
        }
        const SDL2pp::Point out = ctx.r.GetOutputSize();
        scale_x = window_w > 0 ? static_cast<float>(out.x) / static_cast<float>(window_w) : 1.0f;
        scale_y = window_h > 0 ? static_cast<float>(out.y) / static_cast<float>(window_h) : 1.0f;
        buttons = std::move(new_buttons);
    }

    std::vector<bool> hovered_buttons() const {
        int mx = 0, my = 0;
        SDL_GetMouseState(&mx, &my);
        const int x = static_cast<int>(mx * scale_x);
        const int y = static_cast<int>(my * scale_y);

        std::vector<bool> states(buttons.size(), false);
        for (std::size_t i = 0; i < buttons.size(); ++i) {
            states[i] = inside(buttons[i], x, y);
        }
        return states;
    }

    virtual Focus on_button(std::size_t index, Out& out) = 0;
    virtual Focus on_key(Key, Out&) { return Focus::Keep; }

public:
    Focus handle(const InputEvent& e, Out& out) override {
        switch (e.type) {
            case InputEvent::Type::KeyDown:
                return e.repeat ? Focus::Keep : on_key(e.key, out);
            case InputEvent::Type::MouseDown:
                if (e.button == MouseButton::Left) {
                    const int x = static_cast<int>(e.x * scale_x);
                    const int y = static_cast<int>(e.y * scale_y);
                    for (std::size_t i = 0; i < buttons.size(); ++i) {
                        if (inside(buttons[i], x, y)) {
                            return on_button(i, out);
                        }
                    }
                }
                break;
            default: break;
        }
        return Focus::Keep;
    }
};