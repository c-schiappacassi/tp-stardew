#pragma once
#include <map>
#include <string>
#include "protocol_types.h"
#include "../player_view.h"
#include "../../render_info.h"
#include "../../../state.h"

class HotbarView
{
private:
    std::map<uint8_t, Stack> items;
    SpriteId hud_sprite_sheet;
    uint8_t current_slot;

    void draw_bar(const RenderInfo& ctx, const SDL2pp::Rect& dest, int s);
    void drawStack(const RenderInfo& ctx, const Stack& stack, const SDL2pp::Point& slot_pos, int slot_px, int s);
    void draw_slot_label(const RenderInfo& ctx, uint8_t slot, const SDL2pp::Point& slot_pos, int s);
public:
    explicit HotbarView() : hud_sprite_sheet(SpriteId::InGameUI){};

    void update(const PlayerData& player_data){
        //items = player_data.items;
        //current_slot = player_data.current_slot;
    };

    void draw(const RenderInfo& ctx);
};