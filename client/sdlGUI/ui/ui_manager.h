#pragma once

#include <cctype>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <algorithm>
#include <vector>

#include "input_event.h"
#include "ui_layer.h"
#include "world_input.h"

#include "layers/chat_layer.h"
#include "layers/hud_layer.h"
#include "layers/minimap_layer.h"
#include "layers/quit_layer.h"
#include "layers/ready_to_sleep_layer.h"
#include "layers/night_sky_layer.h"

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

    template <class Layer, class... Args>
    Layer& emplace(Args&&... args) {
        auto layer = std::make_unique<Layer>(std::forward<Args>(args)...);
        Layer& ref = *layer;
        layers.push_back(std::move(layer));
        return ref;
    }

    void bind_open(Key key, UiLayer& layer) { openers[key] = &layer; }

public:
    explicit UiManager(const std::string& player_name): world(player_name) {
        emplace<HudLayer>();

        NightSkyLayer& sky = emplace<NightSkyLayer>(); 

        MinimapLayer& minimap = emplace<MinimapLayer>();
        bind_open(Key::M, minimap);

        ChatLayer& chat = emplace<ChatLayer>(player_name);
        bind_open(Key::T, chat);

        QuitLayer& quit_layer = emplace<QuitLayer>(player_name);
        bind_open(Key::Escape, quit_layer);
        
        ReadyToSleepLayer& sleep = emplace<ReadyToSleepLayer>(player_name);
        sky.watch(sleep);
    }

    UiManager(const UiManager&) = delete;
    UiManager& operator=(const UiManager&) = delete;


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

    void draw(const RenderInfo& ctx) {
        for (auto& layer: layers) {
            if (layer->visible()) {
                layer->draw(ctx);
            }
        }
    }

    bool input_captured() const { return focused != nullptr; }

    bool quit_requested() const {
        return quit || std::any_of(layers.begin(), layers.end(),
                [](const auto& layer) { return layer->wants_quit(); });
    }

    void update(const State& state) {
        for (auto& layer: layers) {
            layer->update(state);
            switch (layer->focus_wish()) {
                case FocusWish::Acquire: focus(*layer); break;
                case FocusWish::Release: release(*layer); break;
                case FocusWish::None: break;
            }
        }
    }
};
