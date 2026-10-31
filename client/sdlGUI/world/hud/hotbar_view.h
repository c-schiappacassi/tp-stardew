#pragma once
#include <cstdint>
#include <map>

#include <SDL2pp/SDL2pp.hh>

#include "../../../state.h"
#include "../../render_info.h"

#include "protocol_types.h"

class HotbarView {
private:
    std::map<uint8_t, Stack> items;
    SpriteId hud_sprite_sheet;
    uint8_t current_slot = 0;

    void draw_bar(const RenderInfo& ctx, const SDL2pp::Rect& dest, int s);
    void drawStack(const RenderInfo& ctx, const Stack& stack, const SDL2pp::Point& slot_pos, int slot_px, int s);
    void draw_slot_label(const RenderInfo& ctx, uint8_t slot, const SDL2pp::Point& slot_pos, int s);

public:
    explicit HotbarView(): hud_sprite_sheet(SpriteId::InGameUI) {}

    void update(const PlayerDataLocal& local);

    void draw(const RenderInfo& ctx);
};