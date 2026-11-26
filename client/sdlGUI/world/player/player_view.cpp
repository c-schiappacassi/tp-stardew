#include "player_view.h"

namespace {
    int facing_row(Direction dir, int up_row) { return dir == Direction::Up ? up_row : (dir == Direction::Down ? 0 : 1); }

    SDL2pp::Rect shirt_src(uint16_t model, Direction dir) {
        return SDL2pp::Rect((model % farmer::SHIRTS_PER_ROW) * farmer::SHIRT_SIZE, (model / farmer::SHIRTS_PER_ROW) * farmer::SHIRT_BLOCK_H + facing_row(dir, 3) * farmer::SHIRT_SIZE, farmer::SHIRT_SIZE, farmer::SHIRT_SIZE);
    }

    SDL2pp::Rect pants_src(uint16_t model, const SDL2pp::Rect& body) {
        return SDL2pp::Rect(body.x + (model % farmer::PANTS_PER_ROW) * farmer::PANTS_BLOCK_W, body.y + (model / farmer::PANTS_PER_ROW) * farmer::PANTS_BLOCK_H, body.w, body.h);
    }

    SDL2pp::Rect arms_src(const SDL2pp::Rect& body) { return SDL2pp::Rect(body.x + farmer::BODY_REGION_W, body.y, body.w, body.h); }

    Offset add(Offset a, Offset b) { return Offset{static_cast<int8_t>(a.x + b.x), static_cast<int8_t>(a.y + b.y)}; }
}

PlayerView::PlayerView(const PlayerData& data) {
    hit_flash = &health_trigger.add<HitFlash>();
    health_trigger.add<PlayerHurtSound>(sfx);
    step_trigger.add<FootstepSound>(sfx);
    tool_trigger.add<SwordSwingSound>(sfx);
    update(data);
}

void PlayerView::update(const PlayerData& data) {
    sfx.clear();
    pos = data.pos;
    state = data.state;
    name = data.characterName;
    if (data.dir != Direction::None) facing = data.dir;
    hair_model = data.hairModel;
    hair_color = to_sdl(data.hairColor);
    shirt_model = data.shirtModel;
    pant_model = data.pantsModel;
    pant_color = to_sdl(data.pantsColor);
    health_trigger.observe(data.health, data.pos);
    step_trigger.observe(data.state, data.pos, data.currentZone);
    tool_trigger.observe(data.state, data.heldItem, facing, data.pos);
    if (data.heldItem != held_item) clip_player.reset();
    held_item = data.heldItem;
}

std::optional<float> PlayerView::depth(const RenderInfo& ctx) const {
    return depth_if_visible(ctx, pos, farmer::CELL_H);
}

ClipId PlayerView::clip_id(const RenderInfo& ctx) const {
    ClipId id{SpriteId::FarmerBase, Action{state}, facing};
    if (state == PlayerState::UsingTool && held_item) {
        ClipId with_item = id;
        with_item.item_id = held_item;
        if (ctx.assets.has_clip(with_item)) return with_item;
    }
    return id;
}

void PlayerView::draw(const RenderInfo& ctx) {
    sfx.flush(ctx);
    const AnimatedFrame* frame = clip_player.frame(ctx, clip_id(ctx));
    if (!frame) return;

    const Sprite& body = frame->get_sprite();
    const LayerOffsets& offsets = frame->get_offsets();
    const DrawOpts opts{.flip = frame->is_flipped(), .tint = hit_flash->tint(ctx)};
    const SDL2pp::Point feet = ctx.cam.world_to_screen(pos.getX(), pos.getY());

    auto layer = [&](SpriteId id, const SDL2pp::Rect& src, Offset off = {}, std::optional<SDL_Color> tint = std::nullopt) {
        Sprite::from(id, src, body.get_dx() + off.x, body.get_dy() + off.y)
            .draw(ctx, feet, {.flip = opts.flip, .tint = combine_tint(tint, opts.tint)});
    };
    auto arms_and_tool = [&] {
        layer(SpriteId::FarmerBase, arms_src(body.src()));
        if (state == PlayerState::UsingTool && held_item) {
            ctx.assets.get_tool_frame(*held_item, facing, clip_player.frame_index()).draw(ctx, feet, opts.flip);
        }
    };

    const bool back_turned = facing == Direction::Up;
    if (back_turned) arms_and_tool();
    body.draw(ctx, feet, opts);
    layer(SpriteId::Pants, pants_src(pant_model, body.src()), offsets.pants, pant_color);
    layer(SpriteId::Shirts, shirt_src(shirt_model, facing), add(farmer::SHIRT_BASE, offsets.shirt));
    layer(SpriteId::Hairstyles, farmer::hair_src(hair_model, facing_row(facing, 2)), add(farmer::HAIR_BASE, offsets.hair), hair_color);
    if (!back_turned) arms_and_tool();
}

void PlayerView::draw_name(const RenderInfo& ctx) const {
    if (name.empty() || !on_screen(ctx, pos, farmer::CELL_H)) return;

    const SDL2pp::Point feet = ctx.cam.world_to_screen(pos.getX(), pos.getY());
    const SDL2pp::Rect above(feet.x, feet.y - farmer::NAME_OFFSET * ctx.cam.get_zoom(), 0, 0);
    UiBox(ctx, above, UiBox::screen(ctx).get_unit()).text<ChatFont>(name, farmer::NAME_STYLE, Anchor::Bottom);
}

std::optional<lighting::Light> PlayerView::light(const RenderInfo& ctx) const {
    using namespace lighting;
    if (!on_screen(ctx, pos, PLAYER_GLOW_RADIUS)) return std::nullopt;

    const int zoom = ctx.cam.get_zoom();
    const SDL2pp::Point feet = ctx.cam.world_to_screen(pos.getX(), pos.getY());
    return Light{SDL2pp::Point(feet.x, feet.y + PLAYER_GLOW_OFFSET_Y * zoom), PLAYER_GLOW_RADIUS * zoom, PLAYER_GLOW_COLOR, PLAYER_GLOW_INTENSITY};
}
