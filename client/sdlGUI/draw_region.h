#pragma once

#include <algorithm>
#include <cmath>
#include <optional>
#include <string>
#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "render_info.h"
#include "utils.h"
#include "position.h"
#include "world/hud/pixel_font.h"
#include "world/hud/chat_pixel_font.h"


inline SDL2pp::Rect screen_rect(const SDL2pp::Point& screen) { return SDL2pp::Rect(0, 0, screen.x, screen.y); }

class ScopedTint {
        SDL2pp::Texture& texture;
        bool active;

    public:
        ScopedTint(SDL2pp::Texture& t, const std::optional<SDL_Color>& tint): texture(t), active(tint.has_value()) {
            if (active) texture.SetColorMod(tint->r, tint->g, tint->b);
        }
        ~ScopedTint() {
            if (active) texture.SetColorMod(255, 255, 255);
        }
        ScopedTint(const ScopedTint&) = delete;
        ScopedTint& operator=(const ScopedTint&) = delete;
};

struct UiFrame {
    SDL2pp::Point size;
    int u;
    SDL2pp::Rect rect() const { return screen_rect(size); }
};

enum class HAlign { Left, Center, Right };
enum class VAlign { Top, Center, Bottom };

inline void copy_sprite(const RenderInfo& ctx, SpriteId sprite_id, const SDL2pp::Rect& src, const SDL2pp::Rect& dst,  bool flip = false, std::optional<SDL_Color> tint = std::nullopt, double angle = 0.0, const SDL2pp::Optional<SDL2pp::Point>& center = SDL2pp::NullOpt) {
    SDL2pp::Texture& texture = ctx.assets.get_texture(sprite_id);
    ScopedTint scoped(texture, tint);
    ctx.r.Copy(texture, src, dst, flip ? -angle : angle, center, flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
}

inline void draw_scaled(const RenderInfo& ctx, SpriteId sprite_id, const SDL2pp::Rect& src, const SDL2pp::Rect& dst, bool flip = false, std::optional<SDL_Color> tint = std::nullopt) {
    copy_sprite(ctx, sprite_id, src, dst, flip, tint);
}

inline constexpr int UI_REFERENCE_HEIGHT = 720;

inline int centered_offset(int outer, int inner) { return (outer - inner) / 2; }

inline int ui_scale(const SDL2pp::Point& screen) { return std::max(1, screen.y / UI_REFERENCE_HEIGHT); }
inline int ui_scale(const RenderInfo& ctx) { return ui_scale(ctx.r.GetOutputSize()); }

inline SDL2pp::Rect aligned_rect(const SDL2pp::Rect& area, int w, int h, HAlign ha, VAlign va, int margin_x = 0, int margin_y = 0) {
    int x = area.x + centered_offset(area.w, w);
    int y = area.y + centered_offset(area.h, h);
    if (ha == HAlign::Left)   x = area.x + margin_x;
    if (ha == HAlign::Right)  x = area.x + area.w - margin_x - w;
    if (va == VAlign::Top)    y = area.y + margin_y;
    if (va == VAlign::Bottom) y = area.y + area.h - margin_y - h;
    return SDL2pp::Rect(x, y, w, h);
}

inline SDL2pp::Rect centered_rect(const SDL2pp::Rect& area, int w, int h) {
    return aligned_rect(area, w, h, HAlign::Center, VAlign::Center);
}

inline SDL2pp::Rect sprite_rect(const Sprite& s, int x, int y, int scale) {
    return SDL2pp::Rect(x, y, s.width * scale, s.height * scale);
}

inline SDL2pp::Rect sprite_rect( const Sprite& s, const SDL2pp::Point& p, int scale) {
    return sprite_rect(s, p.x, p.y, scale);
}

inline SDL2pp::Point fit_size(int w, int h, int max_w, int max_h) {
    w = std::max(1, w);
    h = std::max(1, h);
    if (static_cast<long long>(max_w) * h <= static_cast<long long>(max_h) * w) {
        return SDL2pp::Point(std::max(1, max_w), std::max(1, static_cast<int>(static_cast<long long>(h) * max_w / w)));
    }
    return SDL2pp::Point(std::max(1, static_cast<int>(static_cast<long long>(w) * max_h / h)), std::max(1, max_h));
}

inline SDL2pp::Rect fit_sprite_in(const Sprite& s, const SDL2pp::Rect& area, int box) {
    const SDL2pp::Point size = fit_size(s.width, s.height, box, box);
    return centered_rect(area, size.x, size.y);
}

inline void draw_sprite_in(const RenderInfo& ctx, const Sprite& s, const SDL2pp::Rect& dst, bool flip = false, std::optional<SDL_Color> tint = std::nullopt) {
    draw_scaled(ctx, s.sprite_id, s.src(), dst, flip, tint);
}

inline void draw_sprite_fit(const RenderInfo& ctx, const Sprite& s, const SDL2pp::Rect& area, int box, bool flip = false, std::optional<SDL_Color> tint = std::nullopt) {
    draw_sprite_in(ctx, s, fit_sprite_in(s, area, box), flip, tint);
}

inline void draw_texture_box(const RenderInfo& ctx, SpriteId sprite_id, const SDL2pp::Rect& src, const SDL2pp::Rect& dst, int corner, int scale) {
    const int c = corner * scale;
    const int sx[4] = {0, corner, src.w - corner, src.w};
    const int sy[4] = {0, corner, src.h - corner, src.h};
    const int dx[4] = {0, c, dst.w - c, dst.w};
    const int dy[4] = {0, c, dst.h - c, dst.h};

    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            const SDL2pp::Rect s(src.x + sx[col], src.y + sy[row], sx[col + 1] - sx[col], sy[row + 1] - sy[row]);
            const SDL2pp::Rect d(dst.x + dx[col], dst.y + dy[row], dx[col + 1] - dx[col], dy[row + 1] - dy[row]);
            draw_scaled(ctx, sprite_id, s, d);
        }
    }
}

struct PixelFont {
    static int width(const std::string& t, int px) { return pixel_font::width(t, px); }
    static int height(int px) { return pixel_font::height(px); }
    static void draw(const RenderInfo& ctx, const std::string& t, int x, int y, int px, const SDL_Color& c) { pixel_font::draw(ctx, t, x, y, px, c); }
};
struct ChatFont {
    static int width(const std::string& t, int px) { return chat_font::width(t, px); }
    static int height(int px) { return chat_font::height(px); }
    static void draw(const RenderInfo& ctx, const std::string& t, int x, int y, int px, const SDL_Color& c) { chat_font::draw(ctx, t, x, y, px, c); }
};

template <class Font = PixelFont>
SDL2pp::Rect text_rect_in(const SDL2pp::Rect& area, const std::string& text, int px, HAlign ha, VAlign va, int margin_x = 0, int margin_y = 0) {
    return aligned_rect(area, Font::width(text, px), Font::height(px), ha, va, margin_x, margin_y);
}

inline SDL2pp::Rect region_dst(const RenderInfo& ctx, const SDL2pp::Rect& src, const SDL2pp::Point& origin, int dx, int dy) {
    const int z = ctx.cam.get_zoom();
    return SDL2pp::Rect(origin.x + dx * z, origin.y + dy * z, src.w * z, src.h * z);
}

inline void draw_scaled_rotated(const RenderInfo& ctx, SpriteId sprite_id, const SDL2pp::Rect& src, const SDL2pp::Rect& dst, double angle, const SDL2pp::Point& pivot_px, bool flip = false, std::optional<SDL_Color> tint = std::nullopt) {
    const int z = dst.w / src.w;
    const int px = flip ? src.w - pivot_px.x : pivot_px.x;
    copy_sprite(ctx, sprite_id, src, dst, flip, tint, angle, SDL2pp::Point(px * z, pivot_px.y * z));
}

inline void draw_region_rotated(const RenderInfo& ctx, SpriteId sprite_id, const SDL2pp::Rect& src, const SDL2pp::Point& origin, int dx, int dy, double angle, const SDL2pp::Point& pivot_px, bool flip = false, std::optional<SDL_Color> tint = std::nullopt) {
    draw_scaled_rotated(ctx, sprite_id, src, region_dst(ctx, src, origin, dx, dy), angle, pivot_px, flip, tint);
}

inline UiFrame ui_frame(const RenderInfo& ctx) {
    const SDL2pp::Point size = ctx.r.GetOutputSize();
    return UiFrame{size, ui_scale(size)};
}

inline SDL2pp::Rect centered_rect(const SDL2pp::Point& screen, int w, int h) {
    return centered_rect(screen_rect(screen), w, h);
}

inline SDL2pp::Rect rect_centered_on(const SDL2pp::Point& center, int w, int h) {
    return SDL2pp::Rect(center.x - w / 2, center.y - h / 2, w, h);
}

inline void draw_ui(const RenderInfo& ctx, UiSpriteId id, const SDL2pp::Rect& dst) {
    draw_sprite_in(ctx, ctx.assets.get_sprite(id), dst);
}

inline bool draw_item_icon(  const RenderInfo& ctx, ItemId item, const SDL2pp::Rect& area, int box) {
    draw_sprite_fit(ctx, ctx.assets.get_sprite(item) , area, box);
    return true;
}

inline void draw_ui_box(const RenderInfo& ctx, UiSpriteId id, const SDL2pp::Rect& dst, int corner, int scale) {
    const Sprite& s = ctx.assets.get_sprite(id);
    draw_texture_box(ctx, s.sprite_id, s.src(), dst, corner, scale);
}

inline void fill_rect(const RenderInfo& ctx, const SDL2pp::Rect& rect, const SDL_Color& c) {
    ctx.r.SetDrawColor(c.r, c.g, c.b, c.a);
    ctx.r.FillRect(rect);
}

inline void fill_rect_blended(const RenderInfo& ctx, const SDL2pp::Rect& rect, const SDL_Color& c) {
    ctx.r.SetDrawBlendMode(SDL_BLENDMODE_BLEND);
    fill_rect(ctx, rect, c);
    ctx.r.SetDrawBlendMode(SDL_BLENDMODE_NONE);
}

inline void dim_screen(const RenderInfo& ctx, Uint8 alpha) {
    fill_rect_blended(ctx, screen_rect(ctx.r.GetOutputSize()), SDL_Color{0, 0, 0, alpha});
}

template <class Font = PixelFont>
void draw_text_shadowed(const RenderInfo& ctx, const std::string& text, int x, int y, int px, const SDL_Color& color, const SDL_Color& shadow, int shadow_offset) {
    Font::draw(ctx, text, x + shadow_offset, y + shadow_offset, px, shadow);
    Font::draw(ctx, text, x, y, px, color);
}

template <class Font = PixelFont>
void draw_text_in(const RenderInfo& ctx, const std::string& text, const SDL2pp::Rect& area, int px, HAlign ha, VAlign va,
                  const SDL_Color& color, int margin_x = 0, int margin_y = 0,
                  std::optional<SDL_Color> shadow = std::nullopt, int shadow_offset = 0) {
    if (text.empty()) return;
    const SDL2pp::Rect r = text_rect_in<Font>(area, text, px, ha, va, margin_x, margin_y);
    if (shadow) draw_text_shadowed<Font>(ctx, text, r.x, r.y, px, color, *shadow, shadow_offset);
    else Font::draw(ctx, text, r.x, r.y, px, color);
}

inline bool on_screen(const RenderInfo& ctx, int world_x, int world_y, int radius_world) {
    return ctx.cam.is_visible(world_x, world_y, radius_world * ctx.cam.get_zoom());
}

inline bool on_screen(const RenderInfo& ctx, const Position& pos, int radius_world) {
    return on_screen(ctx, pos.getX(), pos.getY(), radius_world);
}

inline std::optional<float> depth_if_visible(const RenderInfo& ctx, const Position& pos, int radius_world, int y_offset = 0) {
    if (!on_screen(ctx, pos, radius_world)) {
        return std::nullopt;
    }
    return static_cast<float>(static_cast<int>(pos.getY()) + y_offset);
}

inline void draw_region(const RenderInfo& ctx, SpriteId sprite_id, const SDL2pp::Rect& src, const SDL2pp::Point& origin, int dx = 0, int dy = 0, bool flip = false, std::optional<SDL_Color> tint = std::nullopt) {
    draw_scaled(ctx, sprite_id, src, region_dst(ctx, src, origin, dx, dy), flip, tint);
}

inline void draw_sprite(const RenderInfo& ctx, const Sprite& s, const SDL2pp::Point& origin, bool flip = false, std::optional<SDL_Color> tint = std::nullopt) {
    draw_region(ctx, s.sprite_id, s.src(), origin, s.dx(flip), s.offset_y, flip, tint);
}

inline void draw_sprite_rotated(const RenderInfo& ctx, const Sprite& s, const SDL2pp::Point& origin, double angle, const SDL2pp::Point& pivot_px, bool flip = false, std::optional<SDL_Color> tint = std::nullopt) {
    draw_region_rotated(ctx, s.sprite_id, s.src(), origin, s.dx(flip), s.offset_y, angle, pivot_px, flip, tint);
}

inline void draw_sprite_at(const RenderInfo& ctx, const Sprite& s, const Position& pos, bool flip = false, int dy = 0, std::optional<SDL_Color> tint = std::nullopt) {
    draw_sprite(ctx, s, ctx.cam.world_to_screen(pos.getX(), pos.getY() + dy), flip, tint);
}

inline constexpr int BOB_AMPLITUDE = 2;
inline constexpr double BOB_SPEED = 0.15;

inline int bob_offset(const RenderInfo& ctx, int amplitude = BOB_AMPLITUDE, double speed = BOB_SPEED, double phase = 0.0) {
    return static_cast<int>(std::lround(std::sin(static_cast<double>(ctx.it) * speed + phase) * amplitude));
}

template <class Font = PixelFont>
void draw_text_bobbing(const RenderInfo& ctx, const std::string& text, int x, int y, int px, const SDL_Color& color, const SDL_Color& shadow, int shadow_offset,
                       int amplitude, double phase_step, double speed = BOB_SPEED) {
    for (std::size_t i = 0; i < text.size(); ++i) {
        const std::string glyph(1, text[i]);
        const int glyph_x = x + Font::width(text.substr(0, i + 1), px) - Font::width(glyph, px);
        draw_text_shadowed<Font>(ctx, glyph, glyph_x, y + bob_offset(ctx, amplitude, speed, static_cast<double>(i) * phase_step), px, color, shadow, shadow_offset);
    }
}

template <class Player, class ClipIdT>
auto draw_clip_at(const RenderInfo& ctx, Player& player, const ClipIdT& id, const Position& pos, int dy = 0, std::optional<SDL_Color> tint = std::nullopt) {
    auto* frame = player.frame(ctx, id);
    if (frame) draw_sprite_at(ctx, frame->sprite, pos, frame->flip, dy, tint);
    return frame;
}

template <typename Color>
SDL_Color to_sdl(const Color& color) {
    return SDL_Color{
        static_cast<Uint8>(color.r),
        static_cast<Uint8>(color.g),
        static_cast<Uint8>(color.b),
        255
    };
}