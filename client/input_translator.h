#pragma once

#include <SDL2/SDL.h>
#include <algorithm>
#include <string>
#include <utility>
#include <vector>
#include <unordered_map>
#include <memory>
#include <optional>
#include <functional>
#include "state.h"

#include "dtos.h"

struct EventKey {
    uint32_t event_type;
    int32_t code;

    bool operator==(const EventKey& other) const {
        return event_type == other.event_type && code == other.code;
    }
};

struct EventKeyHash {
    std::size_t operator()(const EventKey& k) const {
        return (std::hash<uint32_t>()(k.event_type) ^ (std::hash<int32_t>()(k.code) << 1));
    }
};

class InputTranslator {
private:
    using ActionHandler = std::function<std::shared_ptr<CommandDTO>(const SDL_Event&)>;

    std::string player_name;
    std::unordered_map<EventKey, ActionHandler, EventKeyHash> actions;
    std::vector<Direction> held;
    Direction last_sent = Direction::None;
    bool quit = false;

    static Direction direction_from_key(SDL_Keycode key) {
        static const std::unordered_map<SDL_Keycode, Direction> keys = {
            {SDLK_w, Direction::Up},    {SDLK_UP, Direction::Up},
            {SDLK_s, Direction::Down},  {SDLK_DOWN, Direction::Down},
            {SDLK_a, Direction::Left},  {SDLK_LEFT, Direction::Left},
            {SDLK_d, Direction::Right}, {SDLK_RIGHT, Direction::Right},
        };
        auto it = keys.find(key);
        return it == keys.end() ? Direction::None : it->second;
    }

    static EventKey key_of(const SDL_Event& e) {
        switch (e.type) {
            case SDL_KEYDOWN:
            case SDL_KEYUP: return EventKey{e.type, e.key.keysym.sym};
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP: return EventKey{e.type, e.button.button};
            default: return EventKey{e.type, 0};
        }
    }

    void release(Direction d) { held.erase(std::remove(held.begin(), held.end(), d), held.end()); }

    void handle_direction_key(const SDL_KeyboardEvent& key) {
        const Direction d = direction_from_key(key.keysym.sym);
        if (d == Direction::None) {
            return;
        }
        release(d);
        if (key.type == SDL_KEYDOWN) {
            held.push_back(d);
        }
    }

    std::shared_ptr<CommandDTO> make_move(Direction dir) const {
        auto dto = std::make_shared<MoveDTO>(player_name);
        dto->dir = dir;
        return dto;
    }

public:
    explicit InputTranslator(std::string name) : player_name(std::move(name)) {
        // TODO: cargar map actions[EventKey{SDL_KEYDOWN, SDLK_e}] = [this](const SDL_Event&) { ... };
    }

    bool quit_requested() const { return quit; }

    std::vector<std::shared_ptr<CommandDTO>> poll() {
        std::vector<std::shared_ptr<CommandDTO>> out;

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
                continue;
            }
            if (event.type == SDL_KEYDOWN && event.key.repeat) {
                continue;
            }
            if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
                handle_direction_key(event.key);
            }
            auto it = actions.find(key_of(event));
            if (it != actions.end()) {
                out.push_back(it->second(event));
            }
        }

        const Direction current = held.empty() ? Direction::None : held.back();
        if (current != last_sent) {
            out.push_back(make_move(current));
            last_sent = current;
        }
        return out;
    }
};