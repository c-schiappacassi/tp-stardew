#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "position.h"
#include "utils.h"

struct RenderInfo;

struct DrawOpts {
    bool flip = false;
    std::optional<SDL_Color> tint = std::nullopt;
    int world_dy = 0;
    std::optional<int> scale = std::nullopt;
    double angle = 0.0;
    SDL2pp::Point pivot{0, 0};
    std::optional<Uint8> alpha = std::nullopt;
};

class ScopedTint {
private:
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

class ScopedAlpha {
private:
    SDL2pp::Texture& texture;
    bool active;

public:
    ScopedAlpha(SDL2pp::Texture& t, const std::optional<Uint8>& alpha): texture(t), active(alpha.has_value()) {
        if (active) texture.SetBlendMode(SDL_BLENDMODE_BLEND).SetAlphaMod(*alpha);
    }
    ~ScopedAlpha() {
        if (active) texture.SetAlphaMod(255);
    }
    ScopedAlpha(const ScopedAlpha&) = delete;
    ScopedAlpha& operator=(const ScopedAlpha&) = delete;
};

class Sprite {
private:
    SpriteId sprite_id;
    uint16_t x, y;
    uint16_t width, height;
    int16_t dx;
    int16_t dy;

public:
    constexpr Sprite(SpriteId id, uint16_t src_x, uint16_t src_y, uint16_t w, uint16_t h,
                     int16_t anchor_dx = 0, int16_t anchor_dy = 0)
        : sprite_id(id), x(src_x), y(src_y), width(w), height(h), dx(anchor_dx), dy(anchor_dy) {}

    static Sprite from(SpriteId id, const SDL2pp::Rect& r, int anchor_dx = 0, int anchor_dy = 0);
    
    SpriteId get_sprite_id() const { return sprite_id; }
    int get_width() const { return width; }
    int get_height() const { return height; }
    int get_dx(bool flip = false) const { return flip ? -(dx + static_cast<int>(width)) : dx; }
    int get_dy() const { return dy; }

    SDL2pp::Rect src() const { return SDL2pp::Rect(x, y, width, height); }
    SDL2pp::Rect rect_at(int px, int py, int scale) const { return SDL2pp::Rect(px, py, width * scale, height * scale); }

    void blit(const RenderInfo& ctx, const SDL2pp::Rect& dst, const DrawOpts& opts = {}) const;
    void draw(const RenderInfo& ctx, const SDL2pp::Point& origin, const DrawOpts& opts = {}) const;
    void draw_at(const RenderInfo& ctx, const Position& world, const DrawOpts& opts = {}) const;
    void draw_box(const RenderInfo& ctx, const SDL2pp::Rect& dst, int corner, int scale) const;
};

class FlippableSprite {
private:
    Sprite sprite;
    bool flip;

public:
    constexpr FlippableSprite(const Sprite& s, bool flipped = false): sprite(s), flip(flipped) {}

    const Sprite& get_sprite() const { return sprite; }
    bool is_flipped() const { return flip; }

    void draw_at(const RenderInfo& ctx, const Position& world, DrawOpts opts = {}) const;
};

class ToolFrame {
private:
    Sprite sprite;
    double angle;
    SDL2pp::Point pivot;

public:
    constexpr ToolFrame(const Sprite& s, double rotation, const SDL2pp::Point& rotation_pivot)
        : sprite(s), angle(rotation), pivot(rotation_pivot) {}

    void draw(const RenderInfo& ctx, const SDL2pp::Point& anchor, bool flip) const;
};

class AnimatedFrame {
private:
    Sprite sprite;
    bool flip;
    LayerOffsets offsets;

public:
    AnimatedFrame(const Sprite& s, bool flipped, const LayerOffsets& layer_offsets = {})
        : sprite(s), flip(flipped), offsets(layer_offsets) {}

    const Sprite& get_sprite() const { return sprite; }
    bool is_flipped() const { return flip; }
    const LayerOffsets& get_offsets() const { return offsets; }

    void draw_at(const RenderInfo& ctx, const Position& world, DrawOpts opts = {}) const;
};

struct ClipFrame {
    AnimatedFrame frame;
    int duration_ms;
};

struct Clip {
    std::vector<ClipFrame> sequence;
    bool loop = false;
};
