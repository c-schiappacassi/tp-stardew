#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <utility>

#include "../ui_layer.h"
#include "../../world/hud/chat_view.h"

class ChatLayer : public UiLayer {
private:
    static constexpr std::size_t MAX_LEN = 100;

    std::string player_name;
    std::string buffer;

    void append(const std::string& utf8) {
        for (const char ch: utf8) {
            const auto c = static_cast<unsigned char>(ch);
            if (c < 32 || c > 126) {
                continue;
            }
            if (buffer.size() >= MAX_LEN) {
                break;
            }
            buffer.push_back(static_cast<char>(c));
        }
    }

    void submit(Out& out) {
        if (buffer.find_first_not_of(' ') == std::string::npos) {
            return;
        }
        auto dto = std::make_shared<ChatDTO>(player_name);
        dto->text = buffer;
        out.push_back(dto);
    }

public:
    explicit ChatLayer(std::string name): player_name(std::move(name)) {}

    const std::string& text() const { return buffer; }

    bool visible() const override { return focused(); }

    void on_focus() override { buffer.clear(); }
    void on_blur() override { buffer.clear(); }

    Focus handle(const InputEvent& e, Out& out) override {
        switch (e.type) {
            case InputEvent::Type::Text: append(e.text); break;
            case InputEvent::Type::KeyDown:
                switch (e.key) {
                    case Key::Escape: return Focus::Release;
                    case Key::Enter:
                        submit(out);
                        return Focus::Release;
                    case Key::Backspace:
                        if (!buffer.empty()) {
                            buffer.pop_back();
                        }
                        break;
                    default: break;
                }
                break;
            default: break;
        }
        return Focus::Keep;
    }

    void draw(const RenderInfo& ctx) override { ChatView{}.draw(ctx, buffer); }
};
