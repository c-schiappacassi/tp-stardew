#pragma once
#include <cstdint>
#include <optional>
#include <vector>

#include <SDL2pp/SDL2pp.hh>

#include "../../../state.h"
#include "../../render_info.h"
#include "slot_view.h"

#include "dtos.h"

class ChestInventoryView {
private:
    static constexpr int COLUMNS = 10;

    std::optional<uint32_t> open_id;
    std::vector<SlotView> slot_views;
    std::optional<uint8_t> selected;

    std::vector<SDL2pp::Rect> slot_rects;

    void close();
    void apply_selection();

public:
    ChestInventoryView() = default;

    void update(const PlayerDataLocal& local);
    void draw(const RenderInfo& ctx);

    bool is_open() const { return open_id.has_value(); }
    std::optional<uint32_t> id() const { return open_id; }

    void select(std::optional<uint8_t> slot);

    std::optional<Stack> stack_at(uint8_t slot) const;

    const std::vector<SDL2pp::Rect>& rects() const;
};
