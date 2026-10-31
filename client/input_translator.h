#pragma once

#include <SDL2/SDL.h>

#include <algorithm>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "dtos.h"
#include "sdlGUI/click_target.h"
#include "state.h"

struct EventKey {
    uint32_t event_type;
    int32_t code;

    bool operator==(const EventKey& other) const {
        return event_type == other.event_type && code == other.code;
    }
};

struct EventKeyHash {
    std::size_t operator()(const EventKey& k) const {
        const std::size_t a = std::hash<uint32_t>()(k.event_type);
        const std::size_t b = std::hash<int32_t>()(k.code);
        return a ^ (b + 0x9e3779b9 + (a << 6) + (a >> 2));
    }
};

class InputTranslator {
private:
    using Out = std::vector<std::shared_ptr<CommandDTO>>;
    using Handler = std::function<void(const SDL_Event&, Out&)>;

    std::string player_name;
    TargetPicker pick;
    std::unordered_map<EventKey, Handler, EventKeyHash> bindings;
    std::vector<Direction> held;
    Direction last_sent = Direction::None;
    bool quit = false;

    static EventKey key_of(const SDL_Event& e) {
        switch (e.type) {
            case SDL_KEYDOWN:
            case SDL_KEYUP: return {e.type, e.key.keysym.sym};
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP: return {e.type, e.button.button};
            case SDL_WINDOWEVENT: return {e.type, e.window.event};
            default: return {e.type, 0};
        }
    }

    void bind(uint32_t type, int32_t code, Handler h) { bindings[{type, code}] = std::move(h); }

    void release(Direction d) { held.erase(std::remove(held.begin(), held.end(), d), held.end()); }

    void bind_system() {
        bind(SDL_QUIT, 0, [this](const SDL_Event&, Out&) { quit = true; });
        // sin foco no llega el KEYUP, la direccion quedaria pegada
        bind(SDL_WINDOWEVENT, SDL_WINDOWEVENT_FOCUS_LOST,
             [this](const SDL_Event&, Out&) { held.clear(); });
    }

    void bind_movement() {
        const std::pair<SDL_Keycode, Direction> keys[] = {
            {SDLK_w, Direction::Up},    {SDLK_UP, Direction::Up},
            {SDLK_s, Direction::Down},  {SDLK_DOWN, Direction::Down},
            {SDLK_a, Direction::Left},  {SDLK_LEFT, Direction::Left},
            {SDLK_d, Direction::Right}, {SDLK_RIGHT, Direction::Right},
        };
        for (const auto& [key, dir]: keys) {
            bind(SDL_KEYDOWN, key, [this, dir](const SDL_Event&, Out&) {
                release(dir);
                held.push_back(dir);
            });
            bind(SDL_KEYUP, key, [this, dir](const SDL_Event&, Out&) { release(dir); });
        }
    }

    void bind_mouse() {
        bind(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, [this](const SDL_Event& e, Out& out) {
            if (auto target = pick(e.button.x, e.button.y)) {
                auto dto = std::make_shared<InteractDTO>(player_name);
                dto->targetType = target->type;
                dto->targetId = target->id;
                out.push_back(dto);
            }
        });
    }

    void emit_move_if_changed(Out& out) {
        const Direction current = held.empty() ? Direction::None : held.back();
        if (current == last_sent) {
            return;
        }
        auto dto = std::make_shared<MoveDTO>(player_name);
        dto->dir = current;
        out.push_back(dto);
        last_sent = current;
    }

public:
    InputTranslator(std::string name, TargetPicker picker):
            player_name(std::move(name)), pick(std::move(picker)) {
        bind_system();
        bind_movement();
        bind_mouse();
    }

    bool quit_requested() const { return quit; }

    Out poll() {
        Out out;
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_KEYDOWN && e.key.repeat) {
                continue;
            }
            auto it = bindings.find(key_of(e));
            if (it != bindings.end()) {
                it->second(e, out);
            }
        }
        emit_move_if_changed(out);
        return out;
    }
};