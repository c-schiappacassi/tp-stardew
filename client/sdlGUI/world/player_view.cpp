#include "player_view.h"

namespace {

template <typename Color>
SDL_Color to_sdl(const Color& c) {
    return SDL_Color{static_cast<Uint8>(c.r), static_cast<Uint8>(c.g), static_cast<Uint8>(c.b), 255};
}

void blit(const RenderInfo& ctx, SpriteId sprite, const SDL2pp::Rect& src, const SDL2pp::Point& origin, int dx, int dy, bool flip, std::optional<SDL_Color> tint) {
    const int z = ctx.cam.get_zoom();
    const SDL2pp::Rect dest(origin.x + dx * z, origin.y + dy * z, src.w * z, src.h * z);

    SDL2pp::Texture& texture = ctx.assets.get_sprite(sprite).texture;
    if (tint) {
        texture.SetColorMod(tint->r, tint->g, tint->b);
    }
    ctx.r.Copy(texture, src, dest, 0.0, SDL2pp::NullOpt, flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
    if (tint) {
        texture.SetColorMod(255, 255, 255);
    }
}

SDL2pp::Rect body_cell(const Frame& f, int block_x = 0, int block_y = 0) {
    return SDL2pp::Rect(block_x + f.col * farmer::CELL_W, block_y + f.row * farmer::CELL_H, farmer::CELL_W, farmer::CELL_H);
}
}

PlayerView::PlayerView(std::string name, const PlayerData& d) : name(std::move(name)) { update(d); }

void PlayerView::update(const PlayerData& d) {
    pos = d.pos;
    state = d.state;
    if (d.dir != Direction::None) {
        facing = d.dir;
    }
    hair_model = d.hairModel;
    hair_color = to_sdl(d.hairColor);
    shirt_model = d.shirtModel;
    pant_model = d.pantsModel;
    pant_color = to_sdl(d.pantsColor);
}

std::optional<float> PlayerView::depth(const RenderInfo& ctx) const {
    const int radius = farmer::CELL_H * ctx.cam.get_zoom();
    if (!ctx.cam.is_visible(pos.getX(), pos.getY(), radius)) {
        return std::nullopt;
    }
    return static_cast<float>(pos.getY());
}

void PlayerView::draw_body(const RenderInfo& ctx, const Cell& cell) {
    blit(ctx, SpriteId::FarmerBase, body_cell(cell.frame), cell.origin, 0, 0, cell.frame.flip_h, std::nullopt);
}

void PlayerView::draw_pants(const RenderInfo& ctx, const Cell& cell) {
    const int bx = (pant_model % farmer::PANTS_PER_ROW) * farmer::PANTS_BLOCK_W;
    const int by = (pant_model / farmer::PANTS_PER_ROW) * farmer::PANTS_BLOCK_H;
    blit(ctx, SpriteId::Pants, body_cell(cell.frame, bx, by), cell.origin, 0, 0, cell.frame.flip_h,
         pant_color);
}

void PlayerView::draw_shirt(const RenderInfo& ctx, const Cell& cell) {
    int dir_offset = 0;
    switch (facing) {
        case Direction::Right: dir_offset = 1; break;
        case Direction::Left: dir_offset = 2; break;
        case Direction::Up: dir_offset = 3; break;
        case Direction::Down:
        case Direction::None: dir_offset = 0; break;
    }
    const SDL2pp::Rect src((shirt_model % farmer::SHIRTS_PER_ROW) * farmer::SHIRT_SIZE, (shirt_model / farmer::SHIRTS_PER_ROW) * farmer::SHIRT_BLOCK_H + dir_offset * farmer::SHIRT_SIZE, farmer::SHIRT_SIZE, farmer::SHIRT_SIZE);
    blit(ctx, SpriteId::Shirts, src, cell.origin, farmer::SHIRT_X, farmer::SHIRT_BELOW_HEAD, false, std::nullopt);
}

void PlayerView::draw_hair(const RenderInfo& ctx, const Cell& cell) {
    int dir_row = 0;
    if (facing == Direction::Right || facing == Direction::Left) {
        dir_row = 1;
    } else if (facing == Direction::Up) {
        dir_row = 2;
    }
    const SDL2pp::Rect src((hair_model % farmer::HAIR_PER_ROW) * farmer::CELL_W, (hair_model / farmer::HAIR_PER_ROW) * farmer::HAIR_BLOCK_H + dir_row * farmer::CELL_H, farmer::CELL_W, farmer::CELL_H);
    blit(ctx, SpriteId::Hairstyles, src, cell.origin, 0, farmer::HAIR_ABOVE_HEAD, facing == Direction::Left, hair_color);
}

void PlayerView::draw_arms(const RenderInfo& ctx, const Cell& cell) {
    blit(ctx, SpriteId::FarmerBase, body_cell(cell.frame, farmer::BODY_REGION_W), cell.origin, 0, 0, cell.frame.flip_h, std::nullopt);
}

void PlayerView::draw(const RenderInfo& ctx) {
    const ClipId clip_id{SpriteId::FarmerBase, state, facing, std::nullopt};
    if (!current_clip || *current_clip != clip_id) {
        anim.reset();
        current_clip = clip_id;
        last_it = ctx.it;
    }

    anim.update(static_cast<float>(ctx.it - last_it) / static_cast<float>(FPS));
    last_it = ctx.it;
    const Frame frame = anim.get_current_frame(ctx.assets.get_clip(clip_id));

    const int z = ctx.cam.get_zoom();
    const SDL2pp::Point tile = ctx.cam.world_to_screen(pos.getX(), pos.getY());
    const Cell cell{SDL2pp::Point(tile.x, tile.y + (TILE_TO_PIXEL - farmer::FEET_Y) * z), frame};

    draw_body(ctx, cell);
    draw_pants(ctx, cell);
    draw_shirt(ctx, cell);
    draw_hair(ctx, cell);
    draw_arms(ctx, cell);
    // TBD: if (state == PlayerState::UsingTool && currentTool) currentTool->draw(ctx);
}
