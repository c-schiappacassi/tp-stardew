#pragma once
#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "../../render_info.h"
#include "../../world/hud/chest_inventory_view.h"
#include "../../world/hud/hotbar_view.h"
#include "modal_layer.h"
#include "state.h"

#include "dtos.h"

class ChestLayer : public ModalLayer {
private:
    std::string player_name;
    ChestInventoryView view;
    std::map<uint8_t, Stack> inventory;

    FocusWish wish = FocusWish::None;
    std::size_t chest_slots = 0;

    void request_close(Out& out) {
        out.push_back(std::make_shared<CloseChestDTO>(player_name));
    }

    void withdraw(uint8_t slot, Out& out) {
        const auto stack = view.stack_at(slot);
        if (!stack || !view.id()) {
            return;
        }
        auto dto = std::make_shared<SelectItemDTO>(player_name);
        dto->chestId = *view.id();
        dto->slot = slot;
        dto->quantity = stack->quantity;
        out.push_back(dto);
    }

    void deposit(uint8_t slot, Out& out) {
        const auto it = inventory.find(slot);
        if (it == inventory.end() || !view.id()) {
            return;
        }
        auto dto = std::make_shared<ItemTransferDTO>(player_name);
        dto->chestId = *view.id();
        dto->slot = slot;
        dto->quantity = it->second.quantity;
        out.push_back(dto);
    }

protected:
    Focus on_key(Key key, Out& out) override {
        if (key == Key::Escape) {
            request_close(out);
            return Focus::Keep;
        }

        const int index = static_cast<int>(key) - static_cast<int>(Key::Num1);
        if (index >= 0 && index < HotbarView::COUNT) {
            deposit(static_cast<uint8_t>(index), out);
        }
        return Focus::Keep;
    }

    Focus on_button(std::size_t index, Out& out) override {
        if (index < chest_slots) {
            withdraw(static_cast<uint8_t>(index), out);
        } else {
            deposit(static_cast<uint8_t>(index - chest_slots), out);
        }
        return Focus::Keep;
    }

public:
    explicit ChestLayer(std::string name): player_name(std::move(name)) {}

    bool visible() const override { return view.is_open(); }
    FocusWish focus_wish() const override { return wish; }

    void update(const State& state) override {
        const PlayerDataLocal& local = state.get_local();
        const bool was_open = view.is_open();

        view.update(local);
        inventory = local.items;

        wish = FocusWish::None;
        if (view.is_open() && !was_open) {
            wish = FocusWish::Acquire;
        } else if (!view.is_open() && was_open) {
            wish = FocusWish::Release;
        }
    }

    void draw(const RenderInfo& ctx) override {
        hits.begin(ctx);
        view.draw(ctx, hits);
        chest_slots = hits.size();
        for (const SDL2pp::Rect& slot : HotbarView::slot_rects(ctx)) hits.add(slot);
    }
};
