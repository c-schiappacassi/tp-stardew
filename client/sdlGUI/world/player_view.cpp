#include "player_view.h"

PlayerView::PlayerView(const PlayerData& data) {
    update(data);

    this->dir_offset_shirt[Direction::Down] = 0;
    this->dir_offset_shirt[Direction::None] = 0;
    this->dir_offset_shirt[Direction::Right] = 1;
    this->dir_offset_shirt[Direction::Left] = 2;
    this->dir_offset_shirt[Direction::Up] = 3;

    this->dir_offset_hair[Direction::Down] = 0;
    this->dir_offset_hair[Direction::None] = 0;
    this->dir_offset_hair[Direction::Right] = 1;
    this->dir_offset_hair[Direction::Left] = 1;
    this->dir_offset_hair[Direction::Up] = 2;
}

void PlayerView::update(const PlayerData& data) {
    pos = data.pos;
    state = data.state;
    if (data.dir != Direction::None) {
        facing = data.dir;
    }
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

void PlayerView::draw_body( const RenderInfo& ctx, const AnimatedFrame& frame, const SDL2pp::Point& origin) {
    draw_region( ctx, SpriteId::FarmerBase, sprite_rect(frame.sprite), origin, 0, 0, frame.flip_h );
}

void PlayerView::draw_pants( const RenderInfo& ctx, const AnimatedFrame& frame, const SDL2pp::Point& origin) {
    Sprite pants_sprite = frame.sprite;
    pants_sprite.col += (pant_model % farmer::PANTS_PER_ROW) * (farmer::PANTS_BLOCK_W / farmer::CELL_W);
    pants_sprite.row += (pant_model / farmer::PANTS_PER_ROW) * (farmer::PANTS_BLOCK_H / farmer::CELL_H);

    const SDL2pp::Rect src = sprite_rect(pants_sprite);

    draw_region( ctx, SpriteId::Pants, src, origin, 0, 0, frame.flip_h, pant_color);
}

void PlayerView::draw_shirt(  const RenderInfo& ctx,  const SDL2pp::Point& origin) {
    const int x = (shirt_model % farmer::SHIRTS_PER_ROW) * farmer::SHIRT_SIZE;
    const int y = (shirt_model / farmer::SHIRTS_PER_ROW) * farmer::SHIRT_BLOCK_H + dir_offset_shirt.at(facing) * farmer::SHIRT_SIZE;

    const SDL2pp::Rect src( x, y, farmer::SHIRT_SIZE, farmer::SHIRT_SIZE);

    draw_region( ctx, SpriteId::Shirts, src, origin, farmer::SHIRT_X, farmer::SHIRT_BELOW_HEAD);
}


void PlayerView::draw_hair( const RenderInfo& ctx, const SDL2pp::Point& origin) {
    const int x =  (hair_model % farmer::HAIR_PER_ROW) *  farmer::CELL_W;
    const int y = (hair_model / farmer::HAIR_PER_ROW) *  farmer::HAIR_BLOCK_H + dir_offset_hair.at(facing)  * farmer::CELL_H;

    const SDL2pp::Rect src( x,  y, farmer::CELL_W, farmer::CELL_H );

    draw_region( ctx, SpriteId::Hairstyles, src,  origin,  0, farmer::HAIR_ABOVE_HEAD, facing == Direction::Left, hair_color);
}


void PlayerView::draw_arms( const RenderInfo& ctx, const AnimatedFrame& frame, const SDL2pp::Point& origin) {
    Sprite arms_sprite = frame.sprite;
    arms_sprite.col += farmer::BODY_REGION_W / farmer::CELL_W;

    draw_region( ctx,SpriteId::FarmerBase, sprite_rect(arms_sprite),origin, 0, 0, frame.flip_h);
}


void PlayerView::draw(const RenderInfo& ctx) {
    const ClipId clip_id{ SpriteId::FarmerBase,Action{state}, facing };

    if (!current_clip.has_value() || *current_clip != clip_id) {
        anim.reset();
        current_clip = clip_id;
        last_it = ctx.it;
    }
    
    const long long delta_frames = ctx.it - last_it;
    const float delta_ms =  static_cast<float>(delta_frames) * TICK_MS;

    anim.update(delta_ms);
    last_it = ctx.it;

    const Clip& clip = ctx.assets.get_clip(clip_id);
    const AnimatedFrame& frame = anim.get_current_frame(clip);

    const int z = ctx.cam.get_zoom();

    const SDL2pp::Point tile =  ctx.cam.world_to_screen(pos.getX(), pos.getY());
    const SDL2pp::Point origin( tile.x, tile.y + (TILE_TO_PIXEL - farmer::FEET_Y) * z );

    const bool is_bobbing = state == PlayerState::Walking && frame.sprite.col != 0;
    const int bob = is_bobbing ? farmer::WALK_BOB : 0;

    const SDL2pp::Point bob_origin(  origin.x,  origin.y + bob * z );

    draw_body(ctx, frame, origin);
    draw_pants(ctx, frame, origin);
    draw_shirt(ctx, bob_origin);
    draw_hair(ctx, bob_origin);
    draw_arms(ctx, frame, origin);

    // TODO:
    // draw_tool
}