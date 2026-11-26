#pragma once
#include <cstddef>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "../../render_info.h"
#include "../hitboxes.h"
#include "../ui_layer.h"

class ModalLayer : public UiLayer {
protected:
    Hitboxes hits;

    virtual Focus on_button(std::size_t index, Out& out) = 0;
    virtual Focus on_key(Key, Out&) { return Focus::Keep; }

public:
    Focus handle(const InputEvent& e, Out& out) override {
        if (e.type == InputEvent::Type::KeyDown) {
            return e.repeat ? Focus::Keep : on_key(e.key, out);
        }
        if (e.type == InputEvent::Type::MouseDown && e.button == MouseButton::Left) {
            if (const auto i = hits.hit(e.x, e.y)) return on_button(*i, out);
        }
        return Focus::Keep;
    }
};