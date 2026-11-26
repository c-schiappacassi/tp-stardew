#include "sprite.h"

#include "render_info.h"

Sprite Sprite::from(SpriteId id, const SDL2pp::Rect& r, int anchor_dx, int anchor_dy) {
    return Sprite(id, static_cast<uint16_t>(r.x), static_cast<uint16_t>(r.y), static_cast<uint16_t>(r.w), static_cast<uint16_t>(r.h), static_cast<int16_t>(anchor_dx), static_cast<int16_t>(anchor_dy));
}

void Sprite::blit(const RenderInfo& ctx, const SDL2pp::Rect& dst, const DrawOpts& opts) const {
    SDL2pp::Texture& texture = ctx.assets.get_texture(sprite_id);
    ScopedTint scoped_tint(texture, opts.tint);
    ScopedAlpha scoped_alpha(texture, opts.alpha);

    SDL2pp::Optional<SDL2pp::Point> center = SDL2pp::NullOpt;
    if (opts.angle != 0.0) {
        const int z = dst.w / width;
        const int px = opts.flip ? width - opts.pivot.x : opts.pivot.x;
        center = SDL2pp::Point(px * z, opts.pivot.y * z);
    }
    ctx.r.Copy(texture, src(), dst, opts.flip ? -opts.angle : opts.angle, center,
               opts.flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
}

void Sprite::draw(const RenderInfo& ctx, const SDL2pp::Point& origin, const DrawOpts& opts) const {
    const int z = opts.scale.value_or(ctx.cam.get_zoom());
    blit(ctx, SDL2pp::Rect(origin.x + get_dx(opts.flip) * z, origin.y + dy * z, width * z, height * z), opts);
}

void Sprite::draw_at(const RenderInfo& ctx, const Position& world, const DrawOpts& opts) const {
    draw(ctx, ctx.cam.world_to_screen(world.getX(), world.getY() + opts.world_dy), opts);
}

void Sprite::draw_box(const RenderInfo& ctx, const SDL2pp::Rect& dst, int corner, int scale) const {
    const int c = corner * scale;
    const int sx[4] = {0, corner, width - corner, width};
    const int sy[4] = {0, corner, height - corner, height};
    const int ox[4] = {0, c, dst.w - c, dst.w};
    const int oy[4] = {0, c, dst.h - c, dst.h};

    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            const SDL2pp::Rect piece(x + sx[col], y + sy[row], sx[col + 1] - sx[col], sy[row + 1] - sy[row]);
            const SDL2pp::Rect out(dst.x + ox[col], dst.y + oy[row], ox[col + 1] - ox[col], oy[row + 1] - oy[row]);
            from(sprite_id, piece).blit(ctx, out);
        }
    }
}

void FlippableSprite::draw_at(const RenderInfo& ctx, const Position& world, DrawOpts opts) const {
    opts.flip = flip;
    sprite.draw_at(ctx, world, opts);
}

void ToolFrame::draw(const RenderInfo& ctx, const SDL2pp::Point& anchor, bool flip) const {
    sprite.draw(ctx, anchor, {.flip = flip, .angle = angle, .pivot = pivot});
}

void AnimatedFrame::draw_at(const RenderInfo& ctx, const Position& world, DrawOpts opts) const {
    opts.flip = flip;
    sprite.draw_at(ctx, world, opts);
}
