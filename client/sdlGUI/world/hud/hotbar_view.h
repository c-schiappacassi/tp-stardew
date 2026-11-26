#pragma once
#include <cstdint>
#include <vector>

#include <SDL2pp/SDL2pp.hh>

#include "../../../state.h"
#include "ui_box.h"
#include "slot_view.h"

#include "protocol_types.h"

class HotbarView {
private:
    std::vector<SlotView> slot_views;

public:
    static constexpr uint8_t COUNT = 9;

    HotbarView();

    void update(const PlayerDataLocal& local);
    void draw(const RenderInfo& ctx);

    static std::vector<SDL2pp::Rect> slot_rects(const RenderInfo& ctx);
};
