#pragma once
#include <algorithm>
#include <cstddef>
#include <string>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "../../draw_region.h"
#include "../../render_info.h"
#include "chat_pixel_font.h"

class ChatView {
private:
    struct Line {
        std::string text;
        std::size_t name_len = 0;
    };

    struct Layout {
        int u;
        int px;
        int line_h;
        int pad;
        std::size_t chars_per_line;
        SDL2pp::Rect log;
        SDL2pp::Rect input;
    };

    static constexpr int REFERENCE_HEIGHT = 720;

    static constexpr int FRAME_SCALE = 3;
    static constexpr int LOG_CORNER = 4;
    static constexpr int INPUT_CORNER = 3;
    static constexpr int INPUT_H = 14 * FRAME_SCALE;

    static constexpr int CHAT_W = 480;
    static constexpr int MARGIN = 8;
    static constexpr int PAD = 14;
    static constexpr int TEXT_PX = 2;
    static constexpr int LINE_GAP = 2;
    static constexpr std::size_t VISIBLE_LINES = 7;
    static constexpr Uint32 CURSOR_BLINK_MS = 500;

    static constexpr SDL_Color TEXT_COLOR{255, 255, 255, 255};

    static int advance(int px) { return (chat_font::GLYPH_W + chat_font::GLYPH_GAP) * px; }

    static Layout make_layout(const SDL2pp::Point& screen) {
        const int u = std::max(1, screen.y / REFERENCE_HEIGHT);
        const int px = TEXT_PX * u;
        const int line_h = chat_font::height(px) + LINE_GAP * u;
        const int pad = PAD * u;

        const int w = std::min(CHAT_W * u, screen.x / 2);
        const int input_h = INPUT_H * u;
        const int log_h = static_cast<int>(VISIBLE_LINES) * line_h + 2 * pad;

        const int x = MARGIN * u;
        const int input_y = screen.y - MARGIN * u - input_h;

        const int inner_w = w - 2 * pad;
        const int fit = (inner_w + chat_font::GLYPH_GAP * px) / advance(px);

        return Layout{u, px, line_h, pad, static_cast<std::size_t>(std::max(1, fit)), SDL2pp::Rect(x, input_y - log_h, w, log_h), SDL2pp::Rect(x, input_y, w, input_h)};
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

    static void draw_text(const RenderInfo& ctx, const std::string& text, int x, int y, int px, const SDL_Color& color) {
        if (text.empty()) {
            return;
        }
        chat_font::draw(ctx, text, x, y, px, color);
    }

    static void draw_line(const RenderInfo& ctx, const Line& line, int x, int y, int px) {
        const int name_w = static_cast<int>(line.name_len) * advance(px);
        draw_text(ctx, line.text.substr(0, line.name_len), x, y, px, TEXT_COLOR); // TODO: TBD si el nombre va en otro color
        draw_text(ctx, line.text.substr(line.name_len), x + name_w, y, px, TEXT_COLOR);
    }

    static void draw_messages(const RenderInfo& ctx, const Layout& l) {
        const auto& history = ctx.state.get_chat_history();

        std::vector<Line> newest_first;
        for (auto msg = history.rbegin(); msg != history.rend() && newest_first.size() < VISIBLE_LINES; ++msg) {
            const std::string prefix = msg->sender + ": ";
            const std::vector<Line> lines = wrap(prefix + msg->text, l.chars_per_line, prefix.size());
            newest_first.insert(newest_first.end(), lines.rbegin(), lines.rend());
        }

        const std::size_t count = std::min(newest_first.size(), VISIBLE_LINES);
        for (std::size_t i = 0; i < count; ++i) {
            const int row = static_cast<int>(VISIBLE_LINES - 1 - i);
            draw_line(ctx, newest_first[i], l.log.x + l.pad, l.log.y + l.pad + row * l.line_h, l.px);
        }
    }

    static void draw_input(const RenderInfo& ctx, const Layout& l, const std::string& input) {
        const std::size_t fit = l.chars_per_line - 1;
        const std::string shown = input.size() > fit ? input.substr(input.size() - fit) : input;

        const int x = l.input.x + l.pad;
        const int y = l.input.y + (l.input.h - chat_font::height(l.px)) / 2;
        draw_text(ctx, shown, x, y, l.px, TEXT_COLOR);

        if ((SDL_GetTicks() / CURSOR_BLINK_MS) % 2 == 0) {
            ctx.r.SetDrawColor(TEXT_COLOR.r, TEXT_COLOR.g, TEXT_COLOR.b, TEXT_COLOR.a);
            ctx.r.FillRect(SDL2pp::Rect(x + static_cast<int>(shown.size()) * advance(l.px), y, l.px,
                                        chat_font::CAP_H * l.px));
        }
    }

public:
    void draw(const RenderInfo& ctx, const std::string& input, bool typing) const {
        const Layout l = make_layout(ctx.r.GetOutputSize());

        const Sprite& log_frame = ctx.assets.get_sprite(UiSpriteId::ChatBox);
        draw_texture_box(ctx, log_frame.sprite_id, log_frame.src(), l.log, LOG_CORNER, FRAME_SCALE * l.u);
        draw_messages(ctx, l);

        if (typing) {
            const Sprite& input_frame = ctx.assets.get_sprite(UiSpriteId::ChatTypeBox);
            draw_texture_box(ctx, input_frame.sprite_id, input_frame.src(), l.input, INPUT_CORNER,
                             FRAME_SCALE * l.u);
            draw_input(ctx, l, input);
        }
    }
};