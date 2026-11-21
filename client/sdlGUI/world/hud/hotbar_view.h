#pragma once
#include <cstdint>
#include <map>

#include <SDL2pp/SDL2pp.hh>

#include "../../../state.h"
#include "../../render_info.h"

#include "protocol_types.h"

class HotbarView {
private:
    std::map<uint8_t, Stack> items; // TODO: cambiar a SlotView para que se reutilize en ChestView
    uint8_t current_slot = 0;
    uint8_t cap = 9;

    void draw_slot(const RenderInfo& ctx, uint8_t slot, const SDL2pp::Rect& slot_rect, int u);
    void draw_stack(const RenderInfo& ctx, const Stack& stack, const SDL2pp::Rect& slot_rect, int u);
public:
    HotbarView() = default;

    void update(const PlayerDataLocal& local);
    void draw(const RenderInfo& ctx);
};
