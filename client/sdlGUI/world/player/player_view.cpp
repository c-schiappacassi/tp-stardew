#include "player_view.h"

namespace {
    SDL2pp::Rect hair_src(uint16_t model, Direction dir) {
        const int row = dir == Direction::Up ? 2 : (dir == Direction::Down ? 0 : 1);
        return farmer::hair_src(model, row);
    }

    SDL2pp::Rect shirt_src(uint16_t model, Direction dir) {
        const int row = dir == Direction::Up ? 3 : (dir == Direction::Down ? 0 : 1);
        return SDL2pp::Rect((model % farmer::SHIRTS_PER_ROW) * farmer::SHIRT_SIZE, (model / farmer::SHIRTS_PER_ROW) * farmer::SHIRT_BLOCK_H + row * farmer::SHIRT_SIZE, farmer::SHIRT_SIZE, farmer::SHIRT_SIZE);
    }

    SDL2pp::Rect pants_src(uint16_t model, const SDL2pp::Rect& body_src) {
        return SDL2pp::Rect(body_src.GetX() + (model % farmer::PANTS_PER_ROW) * farmer::PANTS_BLOCK_W, body_src.GetY() + (model / farmer::PANTS_PER_ROW) * farmer::PANTS_BLOCK_H, body_src.GetW(), body_src.GetH());
    }

    SDL2pp::Rect arms_src(const Sprite& body) {
        return SDL2pp::Rect(body.x + farmer::BODY_REGION_W, body.y, body.width, body.height);
    }

    Offset add(Offset a, Offset b) {
        return Offset{static_cast<int8_t>(a.x + b.x), static_cast<int8_t>(a.y + b.y)};
    }

    inline int layer_x(int x, bool flip, int cell_w, int layer_w) {
        return flip ? (cell_w - layer_w - x) : x;
    }

    void draw_tool_layer(const RenderInfo& ctx, const ToolFrame& tf, const SDL2pp::Point& anchor, bool flip) {
        draw_sprite_rotated(ctx, tf.sprite, anchor, tf.angle, tf.pivot, flip);
    }
}

PlayerView::PlayerView(const PlayerData& data) {
    update(data);
}

void PlayerView::update(const PlayerData& data) {
    pos = data.pos;
    state = data.state;
    name = data.characterName;
    if (data.dir != Direction::None) { facing = data.dir; }
    hair_model = data.hairModel;
    hair_color = to_sdl(data.hairColor);
    shirt_model = data.shirtModel;
    pant_model = data.pantsModel;
    pant_color = to_sdl(data.pantsColor);
    health = data.health;
    if (data.heldItem != held_item) {
        clip_player.reset();
    }
    held_item = data.heldItem;
}

std::optional<float> PlayerView::depth(const RenderInfo& ctx) const {
    return depth_if_visible(ctx, pos, farmer::CELL_H);
}

void PlayerView::draw_layers_in_order(const RenderInfo& ctx, const farmer::DrawPlayerData& d) {
    auto layer = [&](SpriteId id, const SDL2pp::Rect& src, Offset off = {}, std::optional<SDL_Color> tint = std::nullopt) {
        draw_region(ctx, id, src, d.origin, layer_x(off.x, d.flip, d.cell_w, src.w), off.y, d.flip, tint);
    };

    auto tool = [&]() {
        if (state != PlayerState::UsingTool || !held_item) return;
        if (const ToolFrame* tf = ctx.assets.find_tool_frame(*held_item, facing, d.frame_index)) {
            draw_tool_layer(ctx, *tf, d.anchor, d.flip);
        }
    };

    const bool arms_behind = (facing == Direction::Up);
    if (arms_behind) {
        layer(SpriteId::FarmerBase, d.arms);
        tool();
    }

    layer(SpriteId::FarmerBase, d.body_src);
    layer(SpriteId::Pants,      pants_src(pant_model, d.body_src), d.offsets.pants, pant_color);
    layer(SpriteId::Shirts,     shirt_src(shirt_model, facing),    add(farmer::SHIRT_BASE, d.offsets.shirt));
    layer(SpriteId::Hairstyles, hair_src(hair_model, facing),      add(farmer::HAIR_BASE, d.offsets.hair), hair_color);

    if (!arms_behind) {
        layer(SpriteId::FarmerBase, d.arms);
        tool();
    }
}

void PlayerView::draw(const RenderInfo& ctx) {
    ClipId clip_id{SpriteId::FarmerBase, Action{state}, facing};
    if (state == PlayerState::UsingTool && held_item) {
        ClipId with_item = clip_id;
        with_item.item_id = held_item;
        if (ctx.assets.has_clip(with_item)) clip_id = with_item;
    }

    const AnimatedFrame* frame = clip_player.frame(ctx, clip_id);
    if (!frame) return;

    const Sprite& body = frame->sprite;
    const int zoom = ctx.cam.get_zoom();
    const SDL2pp::Point anchor = ctx.cam.world_to_screen(pos.getX(), pos.getY());
    const SDL2pp::Point origin(anchor.GetX() + body.dx(frame->flip) * zoom,
                               anchor.GetY() + body.offset_y * zoom);

    draw_layers_in_order(ctx, farmer::DrawPlayerData{
        anchor,
        origin,
        body.src(),
        arms_src(body),
        frame->offsets,
        clip_player.frame_index(),
        body.width,
        frame->flip
    });
}

void PlayerView::draw_name(const RenderInfo& ctx) const {
    if (name.empty() || !on_screen(ctx, pos, farmer::CELL_H)) return;

    const int zoom = ctx.cam.get_zoom();
    const int px = farmer::NAME_SCALE_UP * ui_scale(ctx);

    const SDL2pp::Point feet = ctx.cam.world_to_screen(pos.getX(), pos.getY());
    const SDL2pp::Rect area(feet.x, feet.y - farmer::NAME_OFFSET * zoom, 0, 0);

    draw_text_in<ChatFont>(ctx, name, area, px, HAlign::Center, VAlign::Bottom, farmer::NAME_COLOR, 0, 0, farmer::NAME_SHADOW, px);
}

std::optional<lighting::Light> PlayerView::light(const RenderInfo& ctx) const {
    const int zoom = ctx.cam.get_zoom();
    const int radius = lighting::PLAYER_GLOW_RADIUS * zoom;
    if (!on_screen(ctx, pos, lighting::PLAYER_GLOW_RADIUS)) return std::nullopt;

    const SDL2pp::Point feet = ctx.cam.world_to_screen(pos.getX(), pos.getY());
    return lighting::Light{
        SDL2pp::Point(feet.x, feet.y + lighting::PLAYER_GLOW_OFFSET_Y * zoom),
        radius,
        lighting::PLAYER_GLOW_COLOR,
        lighting::PLAYER_GLOW_INTENSITY};
}