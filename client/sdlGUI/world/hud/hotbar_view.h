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
    uint8_t current_slot = 0;
    uint8_t capacity = 9;

    void draw_slot(const RenderInfo& ctx, uint8_t slot, const SDL2pp::Point& pos, int u);
    void draw_stack(const RenderInfo& ctx, const Stack& stack, const SDL2pp::Point& pos, int u);

public:
    HotbarView() = default;

    void update(const PlayerDataLocal& local);
    void draw(const RenderInfo& ctx);
};