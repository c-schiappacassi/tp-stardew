#pragma once

#include <algorithm>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "dtos.h"
#include "input_event.h"

class WorldInput {
public:
    using Out = std::vector<std::shared_ptr<CommandDTO>>;

private:
    // TODO: por ahora hardcodeado, hay que implementar la recompensa en el server status y que me pase el cap de hotbar
    static constexpr int HOTBAR_SLOTS = 9;

    std::string player_name;
    std::vector<Direction> held;
    Direction last_sent = Direction::None;

    static std::optional<Direction> direction_of(Key key) {
        switch (key) {
            case Key::W: case Key::Up: return Direction::Up;
            case Key::S: case Key::Down: return Direction::Down;
            case Key::A: case Key::Left: return Direction::Left;
            case Key::D: case Key::Right: return Direction::Right;
            default: return std::nullopt;
        }
    }

    static std::optional<uint8_t> slot_of(Key key) {
        const int index = static_cast<int>(key) - static_cast<int>(Key::Num1);
        if (index < 0 || index >= HOTBAR_SLOTS) {
            return std::nullopt;
        }
        return static_cast<uint8_t>(index);
    }

    Direction current() const { return held.empty() ? Direction::None : held.back(); }

    void release(Direction d) { held.erase(std::remove(held.begin(), held.end(), d), held.end()); }

    void key_down(Key key, Out& out) {
        if (const auto dir = direction_of(key)) {
            release(*dir);
            held.push_back(*dir);
        } else if (const auto slot = slot_of(key)) {
            auto dto = std::make_shared<SelectSlotDTO>(player_name);
            dto->slot = *slot;
            out.push_back(dto);
        }
    }

    void interact(Out& out) {
        auto dto = std::make_shared<InteractDTO>(player_name);
        dto->dir = current();
        out.push_back(dto);
    }

public:
    explicit WorldInput(std::string name): player_name(std::move(name)) {}

    void handle(const InputEvent& e, Out& out) {
        switch (e.type) {
            case InputEvent::Type::KeyDown:
                if (!e.repeat) {
                    key_down(e.key, out);
                }
                break;
            case InputEvent::Type::KeyUp:
                if (const auto dir = direction_of(e.key)) {
                    release(*dir);
                }
                break;
            case InputEvent::Type::MouseDown:
                if (e.button == MouseButton::Left) {
                    interact(out);
                }
                break;
            default: break;
        }
    }

    void release_all() { held.clear(); }

    void end_frame(Out& out) {
        const Direction dir = current();
        if (dir == last_sent) {
            return;
        }
        auto dto = std::make_shared<MoveDTO>(player_name);
        dto->dir = dir;
        out.push_back(dto);
        last_sent = dir;
    }
};
