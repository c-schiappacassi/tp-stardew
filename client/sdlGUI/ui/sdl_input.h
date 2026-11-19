#pragma once

#include <SDL2/SDL.h>

#include <optional>

#include "input_event.h"

namespace sdl_input {

inline Key key_from_sdl(SDL_Keycode sym) {
    switch (sym) {
        case SDLK_UP: return Key::Up;
        case SDLK_DOWN: return Key::Down;
        case SDLK_LEFT: return Key::Left;
        case SDLK_RIGHT: return Key::Right;
        case SDLK_KP_ENTER: return Key::Enter;
        default: break;
    }
    return (sym > 0 && sym < 127) ? static_cast<Key>(sym) : Key::Unknown;
}

inline MouseButton button_from_sdl(Uint8 button) {
    switch (button) {
        case SDL_BUTTON_LEFT: return MouseButton::Left;
        case SDL_BUTTON_MIDDLE: return MouseButton::Middle;
        case SDL_BUTTON_RIGHT: return MouseButton::Right;
        default: return MouseButton::Other;
    }
}

inline std::optional<InputEvent> translate(const SDL_Event& e) {
    switch (e.type) {
        case SDL_QUIT: return InputEvent::quit();
        case SDL_KEYDOWN: return InputEvent::key_down(key_from_sdl(e.key.keysym.sym), e.key.repeat != 0);
        case SDL_KEYUP: return InputEvent::key_up(key_from_sdl(e.key.keysym.sym));
        case SDL_TEXTINPUT: return InputEvent::typed(e.text.text);
        case SDL_MOUSEBUTTONDOWN: return InputEvent::mouse_down(button_from_sdl(e.button.button), e.button.x, e.button.y);
        case SDL_MOUSEBUTTONUP: return InputEvent::mouse_up(button_from_sdl(e.button.button), e.button.x, e.button.y);
        case SDL_MOUSEWHEEL: {
            int amount = e.wheel.y;
            if (e.wheel.direction == SDL_MOUSEWHEEL_FLIPPED) amount = -amount;
            return InputEvent::mouse_wheel(amount);
        }
        case SDL_WINDOWEVENT:
            if (e.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
                return InputEvent::focus_lost();
            }
            return std::nullopt;
        default: return std::nullopt;
    }
}

}
