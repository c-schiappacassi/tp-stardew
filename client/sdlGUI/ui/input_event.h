#pragma once

#include <cstdint>
#include <string>
#include <utility>

enum class Key : int32_t {
    Unknown = 0,

    Backspace = 8,
    Tab = 9,
    Enter = 13,
    Escape = 27,
    Space = 32,

    Num1 = '1', Num2 = '2', Num3 = '3', Num4 = '4', Num5 = '5',
    Num6 = '6', Num7 = '7', Num8 = '8', Num9 = '9',

    A = 'a', D = 'd', M = 'm', S = 's', T = 't', W = 'w',

    Up = 0x100, Down, Left, Right,
};

inline char printable_char(Key key) {
    const auto code = static_cast<int32_t>(key);
    return (code >= 32 && code < 127) ? static_cast<char>(code) : '\0';
}

enum class MouseButton : uint8_t { Left, Middle, Right, Other };

struct InputEvent {
    enum class Type : uint8_t { KeyDown, KeyUp, Text, MouseDown, MouseUp, FocusLost, Quit };

    Type type = Type::Quit;
    Key key = Key::Unknown;
    bool repeat = false;
    std::string text;
    MouseButton button = MouseButton::Left;
    int x = 0;
    int y = 0;

    static InputEvent key_down(Key key, bool repeat = false) {
        InputEvent e;
        e.type = Type::KeyDown;
        e.key = key;
        e.repeat = repeat;
        return e;
    }
    static InputEvent key_up(Key key) {
        InputEvent e;
        e.type = Type::KeyUp;
        e.key = key;
        return e;
    }
    static InputEvent typed(std::string text) {
        InputEvent e;
        e.type = Type::Text;
        e.text = std::move(text);
        return e;
    }
    static InputEvent mouse_down(MouseButton button, int x = 0, int y = 0) {
        InputEvent e;
        e.type = Type::MouseDown;
        e.button = button;
        e.x = x;
        e.y = y;
        return e;
    }
    static InputEvent mouse_up(MouseButton button, int x = 0, int y = 0) {
        InputEvent e;
        e.type = Type::MouseUp;
        e.button = button;
        e.x = x;
        e.y = y;
        return e;
    }
    static InputEvent focus_lost() {
        InputEvent e;
        e.type = Type::FocusLost;
        return e;
    }
    static InputEvent quit() { return InputEvent{}; }
};
