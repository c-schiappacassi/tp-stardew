#pragma once
#include <chrono>
#include <cstddef>
#include <memory>
#include <string>
#include <utility>

#include "../../render_info.h"
#include "../../world/hud/chat_view.h"
#include "../ui_layer.h"
#include "state.h"

class ChatLayer : public UiLayer {
private:
    using Clock = std::chrono::steady_clock;

    static constexpr std::chrono::milliseconds PEEK_DURATION{5000};
    static constexpr std::size_t MAX_INPUT_CHAT = 100;
    static constexpr int WHEEL_LINES = 2;

    std::string player_name;
    ChatView view;

    std::string input;
    std::size_t seen_messages = 0;
    Clock::time_point peek_until{};

    std::size_t scroll = 0;
    std::size_t max_scroll = 0;

    bool peeking() const { return Clock::now() < peek_until; }

    static std::string trim(const std::string& s) {
        const auto first = s.find_first_not_of(' ');
        if (first == std::string::npos) {
            return {};
        }
        const auto last = s.find_last_not_of(' ');
        return s.substr(first, last - first + 1);
    }

    void send(Out& out) {
        const std::string text = trim(input);
        if (text.empty()) {
            return;
        }
        auto dto = std::make_shared<ChatDTO>(player_name);
        dto->text = text;
        out.push_back(dto);
    }

    void append(const std::string& typed) {
        for (const char c: typed) {
            if (input.size() >= MAX_INPUT_CHAT) {
                break;
            }
            if (c >= 32 && c < 127) {
                input.push_back(c);
            }
        }
    }
    
    void scroll_by(int lines) {
        const long next = static_cast<long>(scroll) + lines;
        scroll = static_cast<std::size_t>(std::clamp<long>(next, 0, static_cast<long>(max_scroll)));
    }

public:
    explicit ChatLayer(std::string name): player_name(std::move(name)) {}

    bool visible() const override { return focused() || peeking(); }

    void on_focus() override { input.clear(); scroll = 0; }

    void on_blur() override {
        input.clear();
        scroll = 0;
        peek_until = Clock::time_point{};
    }

    void update(const State& state) override {
        const std::size_t total = state.get_chat_history().size();
        if (total > seen_messages) {
            peek_until = Clock::now() + PEEK_DURATION;
        }
        seen_messages = total;
    }

    Focus handle(const InputEvent& e, Out& out) override {
        switch (e.type) {
            case InputEvent::Type::Text:
                append(e.text);
                break;

            case InputEvent::Type::MouseWheel:
                scroll_by(e.y_wheel * WHEEL_LINES);
                break;

            case InputEvent::Type::KeyDown:
                if (e.key == Key::Backspace) {
                    if (!input.empty()) input.pop_back();
                } else if (e.key == Key::Up) {
                    scroll_by(1);
                } else if (e.key == Key::Down) {
                    scroll_by(-1);
                } else if (!e.repeat) {
                    if (e.key == Key::Escape) return Focus::Release;
                    if (e.key == Key::Enter) { send(out); return Focus::Release; }
                }
                break;

            default: break;
        }
        return Focus::Keep;
    }

    void draw(const RenderInfo& ctx) override {
        max_scroll = view.draw(ctx, input, focused(), scroll);
    }
};