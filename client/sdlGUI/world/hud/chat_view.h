#pragma once
#include <algorithm>
#include <cstddef>
#include <string>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "ui_box.h"

class ChatView {
private:
    struct Line {
        std::string text;
        std::size_t name_len = 0;
    };

    static constexpr int FRAME_SCALE = 3;
    static constexpr int LOG_CORNER = 4;
    static constexpr int INPUT_CORNER = 3;
    static constexpr int INPUT_H = 14 * FRAME_SCALE;

    static constexpr int CHAT_W = 600;
    static constexpr int MARGIN = 8;
    static constexpr int PAD = 14;
    static constexpr int TEXT_PX = 2;
    static constexpr int LINE_GAP = 2;
    static constexpr std::size_t MAX_VISIBLE_LINES = 12;
    static constexpr std::size_t MAX_SCROLLBACK_LINES = 300;
    static constexpr Uint32 CURSOR_BLINK_MS = 500;

    static constexpr int ADVANCE = (chat_font::GLYPH_W + chat_font::GLYPH_GAP) * TEXT_PX;
    static constexpr int LINE_H = chat_font::GLYPH_H * TEXT_PX + LINE_GAP;
    static constexpr TextStyle TEXT{.size = TEXT_PX, .color = {255, 255, 255, 255}};
    static constexpr TextStyle NAME{.size = TEXT_PX, .color = {250, 255, 0, 255}};

    static std::size_t chars_in(int width) {
        return static_cast<std::size_t>(std::max(1, (width + chat_font::GLYPH_GAP * TEXT_PX) / ADVANCE));
    }

    static std::vector<Line> wrap(const std::string& text, std::size_t width, std::size_t name_len) {
        std::vector<Line> lines;
        std::size_t pos = 0;

        while (pos < text.size()) {
            if (!lines.empty()) {
                while (pos < text.size() && text[pos] == ' ') {
                    ++pos;
                }
                if (pos >= text.size()) {
                    break;
                }
            }

            std::size_t end = std::min(pos + width, text.size());
            if (end < text.size()) {
                const std::size_t space = text.rfind(' ', end);
                if (space != std::string::npos && space > pos) {
                    end = space;
                }
            }

            std::string chunk = text.substr(pos, end - pos);
            const std::size_t last = chunk.find_last_not_of(' ');
            chunk.erase(last == std::string::npos ? 0 : last + 1);

            Line line;
            line.name_len = lines.empty() ? std::min(name_len, chunk.size()) : 0;
            line.text = std::move(chunk);
            lines.push_back(std::move(line));
            pos = end;
        }
        return lines;
    }

    static void draw_line(const UiBox& row, const Line& line) {
        row.text<ChatFont>(line.text.substr(0, line.name_len), NAME, Anchor::TopLeft);
        row.text<ChatFont>(line.text.substr(line.name_len), TEXT, Anchor::TopLeft, static_cast<int>(line.name_len) * ADVANCE);
    }

    static std::size_t draw_messages(const RenderInfo& ctx, const UiBox& log, int w, std::size_t visible, std::size_t scroll) {
        const std::size_t width = chars_in(w - 2 * PAD);

        std::vector<Line> newest_first;
        const auto& history = ctx.state.get_chat_history();
        for (auto msg = history.rbegin(); msg != history.rend() && newest_first.size() < MAX_SCROLLBACK_LINES; ++msg) {
            const std::string prefix = msg->sender + ": ";
            const std::vector<Line> lines = wrap(prefix + msg->text, width, prefix.size());
            newest_first.insert(newest_first.end(), lines.rbegin(), lines.rend());
        }

        const std::size_t max_scroll = newest_first.size() > visible ? newest_first.size() - visible : 0;
        scroll = std::min(scroll, max_scroll);
        const std::size_t count = std::min(newest_first.size() - scroll, visible);
        for (std::size_t i = 0; i < count; ++i) {
            draw_line(log.sub(PAD, PAD + static_cast<int>(visible - 1 - i) * LINE_H, w - 2 * PAD, LINE_H), newest_first[scroll + i]);
        }
        return max_scroll;
    }

    static void draw_input(const UiBox& box, int w, const std::string& input) {
        const UiBox line = box.place(w - 2 * PAD, chat_font::height(TEXT_PX));
        const std::size_t fit = chars_in(w - 2 * PAD) - 1;
        const std::string shown = input.size() > fit ? input.substr(input.size() - fit) : input;

        line.text<ChatFont>(shown, TEXT, Anchor::TopLeft);
        if ((SDL_GetTicks() / CURSOR_BLINK_MS) % 2 == 0) {
            line.sub(static_cast<int>(shown.size()) * ADVANCE, 0, TEXT_PX, chat_font::CAP_H * TEXT_PX).fill(TEXT.color);
        }
    }

public:
    std::size_t draw(const RenderInfo& ctx, const std::string& input, bool typing, std::size_t scroll) const {
        const UiBox screen = UiBox::screen(ctx);
        const SDL2pp::Rect& out = screen.get_rect();
        const int w = std::min(CHAT_W, out.w / screen.get_unit() / 2);
        const int fit_lines = std::max(1, (out.h / screen.get_unit() / 2 - 2 * PAD) / LINE_H);
        const std::size_t visible = std::min(MAX_VISIBLE_LINES, static_cast<std::size_t>(fit_lines));
        const int log_h = static_cast<int>(visible) * LINE_H + 2 * PAD;

        const UiBox chat = screen.place(w, log_h + INPUT_H, Anchor::BottomLeft, MARGIN, MARGIN);
        const UiBox log = chat.sub(0, 0, w, log_h);
        log.frame(UiSpriteId::ChatBox, LOG_CORNER, FRAME_SCALE);
        const std::size_t max_scroll = draw_messages(ctx, log, w, visible, scroll);

        if (typing) {
            const UiBox box = chat.sub(0, log_h, w, INPUT_H);
            box.frame(UiSpriteId::ChatTypeBox, INPUT_CORNER, FRAME_SCALE);
            draw_input(box, w, input);
        }
        return max_scroll;
    }
};