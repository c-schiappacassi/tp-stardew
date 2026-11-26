#pragma once

#include <optional>
#include <string>
#include <type_traits>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "../../draw_helpers.h"
#include "../../render_info.h"
#include "../../sprite.h"
#include "chat_pixel_font.h"
#include "pixel_font.h"

enum class Anchor { TopLeft, Top, TopRight, Left, Center, Right, BottomLeft, Bottom, BottomRight };

struct TextStyle {
    int size = 1;
    SDL_Color color{255, 255, 255, 255};
    std::optional<SDL_Color> shadow = std::nullopt;
    int shadow_offset = 0;
};

class PixelFont {
public:
    static int width(const std::string& t, int px) { return pixel_font::width(t, px); }
    static int height(int px) { return pixel_font::height(px); }
    static void draw(const RenderInfo& ctx, const std::string& t, int x, int y, int px, const SDL_Color& c) {
        pixel_font::draw(ctx, t, x, y, px, c);
    }
};

class ChatFont {
public:
    static int width(const std::string& t, int px) { return chat_font::width(t, px); }
    static int height(int px) { return chat_font::height(px); }
    static void draw(const RenderInfo& ctx, const std::string& t, int x, int y, int px, const SDL_Color& c) {
        chat_font::draw(ctx, t, x, y, px, c);
    }
};

SDL2pp::Rect anchored_rect(const SDL2pp::Rect& area, int w, int h, Anchor a, int mx = 0, int my = 0);

class UiBox {
private:
    const RenderInfo& ctx;
    SDL2pp::Rect rect;
    int unit;

    template <class S> const Sprite& sprite_ref(const S& s) const {
        if constexpr (std::is_same_v<S, Sprite>) return s;
        else return ctx.assets.get_sprite(s);
    }
    UiBox scaled_to(const Sprite& s, bool cover, const DrawOpts& opts) const;

public:
    UiBox(const RenderInfo& context, const SDL2pp::Rect& r, int u = 1): ctx(context), rect(r), unit(u) {}
    static UiBox screen(const RenderInfo& ctx);

    const SDL2pp::Rect& get_rect() const { return rect; }
    int get_unit() const { return unit; }

    UiBox sub(int x, int y, int w, int h) const;
    UiBox inset(int dx, int dy) const;
    UiBox place(int w, int h, Anchor a = Anchor::Center, int mx = 0, int my = 0) const;

    void fill(const SDL_Color& c) const;

    template <class S> void blit(const S& s, const DrawOpts& opts = {}) const { sprite_ref(s).blit(ctx, rect, opts); }
    template <class S> UiBox fit(const S& s, const DrawOpts& opts = {}) const { return scaled_to(sprite_ref(s), false, opts); }
    template <class S> UiBox cover(const S& s, const DrawOpts& opts = {}) const { return scaled_to(sprite_ref(s), true, opts); }
    template <class S> void frame(const S& s, int corner, int k = 1) const { sprite_ref(s).draw_box(ctx, rect, corner, unit * k); }
    template <class S>
    UiBox sprite(const S& s, int k, Anchor a = Anchor::Center, int mx = 0, int my = 0, const DrawOpts& opts = {}) const {
        const Sprite& spr = sprite_ref(s);
        const UiBox box = place(spr.get_width() * k, spr.get_height() * k, a, mx, my);
        spr.blit(ctx, box.rect, opts);
        return UiBox(ctx, box.rect, unit * k);
    }

    template <class Font = PixelFont>
    void text(const std::string& t, const TextStyle& style, Anchor a = Anchor::Center, int mx = 0, int my = 0) const {
        if (t.empty()) return;
        const int px = style.size * unit;
        const SDL2pp::Rect r = anchored_rect(rect, Font::width(t, px), Font::height(px), a, mx * unit, my * unit);
        if (style.shadow) {
            const int d = style.shadow_offset > 0 ? style.shadow_offset * unit : std::max(1, px / 2);
            Font::draw(ctx, t, r.x + d, r.y + d, px, *style.shadow);
        }
        Font::draw(ctx, t, r.x, r.y, px, style.color);
    }
};