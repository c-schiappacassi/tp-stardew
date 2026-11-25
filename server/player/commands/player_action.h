#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <optional>
#include <utility>
#include <variant>
#include <vector>

#include "dtos.h"

class Inventory;

namespace player_action {
struct OpenChest {
    uint32_t id;
};
struct CloseChest {};
struct TransferItemsTo {
    std::reference_wrapper<Inventory> destination;
    uint8_t slot;
    uint16_t quantity;
};
struct TransferItemsFrom {
    std::reference_wrapper<Inventory> source;
    uint8_t slot;
    uint16_t quantity;
};
struct FaceDirection {
    Direction direction;
};
struct FaceTarget {
    Position target;
};
struct ChangeLocation {
    Zone zone;
    uint8_t level;
    Position position;
    std::optional<InteriorLocation> interior = std::nullopt;
};
struct RestoreLocation {
    Zone zone;
    uint8_t level;
    Position position;
    std::optional<InteriorLocation> interior = std::nullopt;
};
struct UseTool {
    std::chrono::steady_clock::time_point now;
};
struct PerformAction {
    uint16_t energyCost;
    std::chrono::milliseconds interval;
    std::chrono::steady_clock::time_point now;
};
struct Sleep {
    std::optional<Position> bed = std::nullopt;
};
struct SetVitals {
    uint16_t health;
    uint16_t energy;
};
struct RestoreVitals {};
struct ApplyCombatBonus {};
struct ReceiveDamage {
    uint16_t damage;
    std::chrono::steady_clock::time_point now;
};
struct AddGold {
    uint32_t amount;
};
struct SpendGold {
    uint32_t amount;
};
struct AddItem {
    Stack stack;
};
struct AddItemUpTo {
    Stack stack;
};
struct RemoveItem {
    uint8_t slot;
    uint16_t quantity;
};
struct ClearSlot {
    uint8_t slot;
};
struct RemoveItems {
    std::vector<std::pair<uint8_t, uint16_t>> quantities;
};
struct BuyItem {
    Stack stack;
    uint32_t price;
};
struct SellItems {
    std::vector<std::pair<uint8_t, uint16_t>> quantities;
    uint32_t price;
};
}  // namespace player_action

using PlayerAction =
        std::variant<player_action::FaceDirection, player_action::FaceTarget,
                     player_action::ChangeLocation, player_action::RestoreLocation,
                     player_action::UseTool, player_action::PerformAction, player_action::Sleep,
                     player_action::SetVitals,
                     player_action::RestoreVitals, player_action::ApplyCombatBonus,
                     player_action::ReceiveDamage, player_action::AddGold, player_action::SpendGold,
                     player_action::AddItem, player_action::AddItemUpTo, player_action::RemoveItem,
                     player_action::RemoveItems, player_action::ClearSlot, player_action::BuyItem, player_action::SellItems,
                     player_action::OpenChest, player_action::CloseChest, player_action::TransferItemsTo, player_action::TransferItemsFrom>;

struct PlayerActionResult {
    bool applied = false;
    uint32_t amount = 0;
};
