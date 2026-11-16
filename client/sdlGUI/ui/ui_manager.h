#pragma once

#include <cctype>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "input_event.h"
#include "ui_layer.h"
#include "world_input.h"

class UiManager {
public:
    using Out = UiLayer::Out;

private:
    WorldInput world;
    std::vector<std::unique_ptr<UiLayer>> layers;
    std::map<Key, UiLayer*> openers;
    UiLayer* focused = nullptr;

    char swallow = 0;
    bool quit = false;

    void route_to_focused(const InputEvent& e, Out& out) {
        if (e.type == InputEvent::Type::Text && swallow != 0) {
            const bool is_opener = e.text.size() == 1 &&
                    std::tolower(static_cast<unsigned char>(e.text[0])) == swallow;
            swallow = 0;
            if (is_opener) {
                return;
            }
        }
        if (focused->handle(e, out) == Focus::Release) {
            release_focus();
        }
    }

public:
    explicit UiManager(std::string player_name): world(std::move(player_name)) {}

    UiManager(const UiManager&) = delete;
    UiManager& operator=(const UiManager&) = delete;

    template <class Layer, class... Args>
    Layer& emplace(Args&&... args) {
        auto layer = std::make_unique<Layer>(std::forward<Args>(args)...);
        Layer& ref = *layer;
        layers.push_back(std::move(layer));
        return ref;
    }

    void bind_open(Key key, UiLayer& layer) { openers[key] = &layer; }

    void focus(UiLayer& layer) {
        if (focused == &layer) {
            return;
        }
        release_focus();
        focused = &layer;
        layer.has_focus = true;
        world.release_all();
        layer.on_focus();
    }

    void release_focus() {
        if (focused == nullptr) {
            return;
        }
        UiLayer* layer = focused;
        focused = nullptr;
        layer->has_focus = false;
        layer->on_blur();
    }

    void release(UiLayer& layer) {
        if (focused == &layer) {
            release_focus();
        }
    }

    bool input_captured() const { return focused != nullptr; }

    bool quit_requested() const { return quit; }

    void dispatch(const InputEvent& e, Out& out) {
        switch (e.type) {
            case InputEvent::Type::Quit: quit = true; return;
            case InputEvent::Type::FocusLost: world.release_all(); return;
            default: break;
        }

        if (focused != nullptr) {
            route_to_focused(e, out);
            return;
        }

        if (e.type == InputEvent::Type::KeyDown && !e.repeat) {
            const auto opener = openers.find(e.key);
            if (opener != openers.end()) {
                focus(*opener->second);
                swallow = printable_char(e.key);
                return;
            }
        }

        world.handle(e, out);
    }

    void end_frame(Out& out) {
        world.end_frame(out);
        swallow = 0;
    }

    void update(const State& state) {
        for (auto& layer: layers) {
            layer->update(state);
        }
    }

    void draw(const RenderInfo& ctx) {
        for (auto& layer: layers) {
            if (layer->visible()) {
                layer->draw(ctx);
            }
        }
    }

    void request_quit() { quit = true; }
};
