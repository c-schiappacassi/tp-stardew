#include <SDL2/SDL.h>
#include <unordered_map>
#include <variant>
#include <optional>

#include "gameDTO.h" // MOCK

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
    using ActionHandler = GameDTO (*)(const SDL_Event&);
    std::unordered_map<EventKey, ActionHandler, EventKeyHash> actions;

public:
    InputTranslator() {
        // TODO: cargar el map, preguntar a Abriñ si va a otro archivo o aca mismo ponemos todos los eventos
    }

    std::optional<GameDTO> try_pop() {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            EventKey key{event.type, 0};

            if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
                key.code = event.key.keysym.sym;
            } 
            else if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP) {
                key.code = event.button.button;
            }

            auto it = actions.find(key);
            if (it != actions.end()) {
                return it->second(event);
            }
        }

        return std::nullopt;
    }
};

