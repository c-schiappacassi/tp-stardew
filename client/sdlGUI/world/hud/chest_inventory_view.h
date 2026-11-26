#pragma once
#include <cstdint>
#include <optional>
#include <vector>

#include <SDL2pp/SDL2pp.hh>

#include "../../../state.h"
#include "../../ui/hitboxes.h"
#include "ui_box.h"
#include "slot_view.h"

#include "dtos.h"

class ChestInventoryView {
private:
    static constexpr int COLUMNS = 10;

    std::optional<uint32_t> open_id;
    std::vector<SlotView> slot_views;

public:
    ChestInventoryView() = default;

    void update(const PlayerDataLocal& local);

    void draw(const RenderInfo& ctx, Hitboxes& hits);

    bool is_open() const { return open_id.has_value(); }
    std::optional<uint32_t> id() const { return open_id; }

    std::optional<Stack> stack_at(uint8_t slot) const;
};
