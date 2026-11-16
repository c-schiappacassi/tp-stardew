#pragma once

#include <memory>
#include <string>
#include <utility>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "../../render_info.h"
#include "../../world/hud/ready_to_sleep_view.h"
#include "../ui_layer.h"
#include "../ui_manager.h"
#include "state.h"

class ReadyToSleepLayer: public UiLayer {
private:
    UiManager& ui;
    std::string player_name;
    ReadyToSleepView view;

    bool open = false;
    bool confirmed = false;

    SDL2pp::Rect ok_rect{0, 0, 0, 0};
    float scale_x = 1.0f;
    float scale_y = 1.0f;

    static bool inside(const SDL2pp::Rect& r, int x, int y) {
        return x >= r.x && x < r.x + r.w && y >= r.y && y < r.y + r.h;
    }

    bool over_ok(int window_x, int window_y) const {
        return !confirmed && inside(ok_rect, static_cast<int>(window_x * scale_x), static_cast<int>(window_y * scale_y));
    }

    void update_mouse_scale(const RenderInfo& ctx) {
        int window_w = 0;
        int window_h = 0;
        if (SDL_Window* window = SDL_RenderGetWindow(ctx.r.Get())) {
            SDL_GetWindowSize(window, &window_w, &window_h);
        }
        const SDL2pp::Point out = ctx.r.GetOutputSize();
        scale_x = window_w > 0 ? static_cast<float>(out.x) / static_cast<float>(window_w) : 1.0f;
        scale_y = window_h > 0 ? static_cast<float>(out.y) / static_cast<float>(window_h) : 1.0f;
    }

    void confirm(Out& out) {
        if (confirmed) {
            return;
        }
        confirmed = true;
        out.push_back(std::make_shared<DayOkDTO>(player_name));
    }

public:
    ReadyToSleepLayer(UiManager& ui, std::string name): ui(ui), player_name(std::move(name)) {}

    bool visible() const override { return open; }

    void update(const State& state) override {
        const bool required = state.day_confirmation_required();
        if (required && !open) {
            open = true;
            confirmed = false;
            ui.focus(*this);
        } else if (!required && open) {
            open = false;
            confirmed = false;
            ui.release(*this);
        }
    }

    Focus handle(const InputEvent& e, Out& out) override {
        switch (e.type) {
            case InputEvent::Type::KeyDown:
                if (!e.repeat && (e.key == Key::Enter || e.key == Key::Space)) {
                    confirm(out);
                }
                break;
            case InputEvent::Type::MouseDown:
                if (e.button == MouseButton::Left && over_ok(e.x, e.y)) {
                    confirm(out);
                }
                break;
            default: break;
        }
        // TODO: ver si le damos al cliente la opcion de esc del mensaje de dormir
        return Focus::Keep;
    }

    void draw(const RenderInfo& ctx) override {
        update_mouse_scale(ctx);

        const ReadyToSleepView::Layout layout = ReadyToSleepView::layout(ctx, confirmed);
        ok_rect = layout.ok;

        int mouse_x = 0;
        int mouse_y = 0;
        SDL_GetMouseState(&mouse_x, &mouse_y);

        view.draw(ctx, layout, confirmed, over_ok(mouse_x, mouse_y));
    }
};