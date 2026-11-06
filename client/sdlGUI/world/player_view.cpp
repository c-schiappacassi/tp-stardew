#include "player_view.h"

namespace {
    SDL2pp::Rect hair_src(uint16_t model, Direction dir) {
        const int row = dir == Direction::Up ? 2 : (dir == Direction::Down ? 0 : 1);
        return SDL2pp::Rect((model % farmer::HAIR_PER_ROW) * farmer::CELL_W, (model / farmer::HAIR_PER_ROW) * farmer::HAIR_BLOCK_H + row * farmer::CELL_H, farmer::CELL_W, farmer::CELL_H);
    }

    SDL2pp::Rect shirt_src(uint16_t model, Direction dir) {
        const int row = dir == Direction::Up ? 3 : (dir == Direction::Down ? 0 : 1);
        return SDL2pp::Rect((model % farmer::SHIRTS_PER_ROW) * farmer::SHIRT_SIZE, (model / farmer::SHIRTS_PER_ROW) * farmer::SHIRT_BLOCK_H + row * farmer::SHIRT_SIZE, farmer::SHIRT_SIZE, farmer::SHIRT_SIZE);
    }

    SDL2pp::Rect pants_src(uint16_t model, const SDL2pp::Rect& body_src) {
        return SDL2pp::Rect(body_src.GetX() + (model % farmer::PANTS_PER_ROW) * farmer::PANTS_BLOCK_W, body_src.GetY() + (model / farmer::PANTS_PER_ROW) * farmer::PANTS_BLOCK_H, body_src.GetW(), body_src.GetH());
    }

    SDL2pp::Rect arms_src(const Sprite& body) {
        return SDL2pp::Rect((body.col + farmer::COLS) * body.width, body.row * body.height, body.width, body.height);
    }

    Offset add(Offset a, Offset b) {
        return Offset{static_cast<int8_t>(a.x + b.x), static_cast<int8_t>(a.y + b.y)};
    }

    inline int layer_x(int x, bool flip, int cell_w, int layer_w) {
        return flip ? (cell_w - layer_w - x) : x;
    }

    void draw_layer(SDL2pp::Renderer& r, SDL2pp::Texture& tex, const SDL2pp::Rect& src, SDL2pp::Point farmer_pos,  Offset off, bool flip, int cell_w, int scale, std::optional<SDL_Color> tint = std::nullopt) {
        const int dx = layer_x(off.x, flip, cell_w, src.GetW());
        SDL2pp::Rect dst(farmer_pos.GetX() + dx * scale, farmer_pos.GetY() + off.y * scale, src.GetW() * scale, src.GetH() * scale);

        if (tint) {
            tex.SetColorMod(tint->r, tint->g, tint->b);
        }

        r.Copy(tex, src, dst, 0.0, SDL2pp::NullOpt, flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);

        if (tint) {
            tex.SetColorMod(255, 255, 255);
        }
    }

}

PlayerView::PlayerView(const PlayerData& data) {
    update(data);
}

void PlayerView::update(const PlayerData& data) {
    pos = data.pos;
    state = data.state;
    if (data.dir != Direction::None) { facing = data.dir;  }
    hair_model = data.hairModel;
    hair_color = to_sdl(data.hairColor);
    shirt_model = data.shirtModel;
    pant_model = data.pantsModel;
    pant_color = to_sdl(data.pantsColor);
}

std::optional<float> PlayerView::depth(const RenderInfo& ctx) const {
    const int radius = farmer::CELL_H * ctx.cam.get_zoom();
    if (!ctx.cam.is_visible(pos.getX(), pos.getY(), radius)) {
        return std::nullopt;
    }
    return static_cast<float>(pos.getY());
}

void PlayerView::draw(const RenderInfo& ctx) {
    const ClipId clip_id{SpriteId::FarmerBase, Action{state}, facing};
    if (!current_clip || !(*current_clip == clip_id)) {
        current_clip = clip_id;
        anim.reset();
    }

    if (last_it >= 0 && ctx.it > last_it) {
        anim.update(static_cast<float>(ctx.it - last_it) * TICK_MS);
    }
    last_it = ctx.it;

    const AnimatedFrame& frame = anim.get_current_frame(ctx.assets.get_clip(clip_id));

    const int zoom   = ctx.cam.get_zoom();
    const int cell_w = frame.sprite.width;

    const SDL2pp::Point screen = ctx.cam.world_to_screen(pos.getX(), pos.getY());
    const SDL2pp::Point origin(screen.GetX() - (cell_w * zoom) / 2, screen.GetY() - farmer::FEET_Y * zoom);

    const SDL2pp::Rect body_src = sprite_rect(frame.sprite);
    const LayerOffsets& lo = frame.offsets;

    bool arms_behind = (this->facing == Direction::Up);
    if (arms_behind){
        draw_layer(ctx.r, ctx.assets.get_texture(SpriteId::FarmerBase), arms_src(frame.sprite), origin, Offset{}, frame.flip, cell_w, zoom);
    }

    draw_layer(ctx.r, ctx.assets.get_texture(SpriteId::FarmerBase), body_src, origin, Offset{}, frame.flip, cell_w, zoom);
    draw_layer(ctx.r, ctx.assets.get_texture(SpriteId::Pants), pants_src(pant_model, body_src), origin, lo.pants, frame.flip, cell_w, zoom, pant_color);
    draw_layer(ctx.r, ctx.assets.get_texture(SpriteId::Shirts), shirt_src(shirt_model, facing), origin, add(farmer::SHIRT_BASE, lo.shirt), frame.flip, cell_w, zoom);
    draw_layer(ctx.r, ctx.assets.get_texture(SpriteId::Hairstyles), hair_src(hair_model, facing), origin, add(farmer::HAIR_BASE, lo.hair), frame.flip, cell_w, zoom, hair_color);
    
    if (!arms_behind){
        draw_layer(ctx.r, ctx.assets.get_texture(SpriteId::FarmerBase), arms_src(frame.sprite), origin, Offset{}, frame.flip, cell_w, zoom);
    }
}