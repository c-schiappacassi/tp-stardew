#include "ground_items.h"

#include "server_constants.h"

#include <algorithm>
#include <limits>
#include <utility>

#include "player/player_manager.h"

namespace {
int64_t squaredDistance(const Position& from, const Position& to) {
    const int64_t dx = static_cast<int64_t>(from.getX()) - to.getX();
    const int64_t dy = static_cast<int64_t>(from.getY()) - to.getY();
    return dx * dx + dy * dy;
}
}  // namespace

bool GroundItems::add(LootData data, EntityIdGenerator& ids) {
    if (data.quantity == 0 || data.pos.getX() < 0 || data.pos.getY() < 0 || data.pos.getX() > std::numeric_limits<uint16_t>::max() ||
        data.pos.getY() > std::numeric_limits<uint16_t>::max()) {
        return false;
    }
    for (auto& [id, current]: items) {
        if (current.pos.getX() == data.pos.getX() && current.pos.getY() == data.pos.getY() && current.kind == data.kind && current.item == data.item &&
            current.quality == data.quality && data.quantity <= std::numeric_limits<uint32_t>::max() - current.quantity) {
            current.quantity += data.quantity;
            return true;
        }
    }
    if (ids.getNextId() == 0 || items.size() == std::numeric_limits<uint16_t>::max()) {
        return false;
    }
    data.id = ids.generate();
    items.emplace(data.id, data);
    return true;
}

bool GroundItems::drop(const Position& position, ItemId item, uint32_t quantity, Quality quality, EntityIdGenerator& ids) {
    return add({0, position, item, quantity, quality, LootKind::Item}, ids);
}

bool GroundItems::dropGold(const Position& position, uint32_t quantity, EntityIdGenerator& ids) {
    return add({0, position, ItemId::Gold, quantity, Quality::None, LootKind::Gold}, ids);
}

bool GroundItems::collect(PlayerManager& players, uint16_t playerId, const std::function<bool(const Position&)>& hasClearPath) {
    const auto player = players.snapshot(playerId);
    if (player.isResting()) {
        return false;
    }
    std::vector<std::pair<int64_t, EntityId>> candidates;
    for (const auto& [id, item]: items) {
        if (player.canReach(item.pos, PICKUP_RANGE) && hasClearPath(item.pos)) {
            candidates.emplace_back(squaredDistance(player.getPosition(), item.pos), id);
        }
    }
    std::sort(candidates.begin(), candidates.end());
    bool collected = false;
    for (const auto& [distance, id]: candidates) {
        auto& item = items.at(id);
        const uint32_t received = item.kind == LootKind::Gold ?
            players.apply(playerId, player_action::AddGold{item.quantity}).amount :
            players
                .apply(playerId,
                    player_action::AddItemUpTo{Stack{item.item, static_cast<uint16_t>(std::min<uint32_t>(item.quantity, std::numeric_limits<uint16_t>::max())), item.quality}})
                .amount;
        if (received == 0) {
            continue;
        }
        item.quantity -= received;
        if (item.quantity == 0) {
            items.erase(id);
        }
        collected = true;
    }
    return collected;
}

std::vector<LootData> GroundItems::getPublicData() const {
    std::vector<LootData> result;
    result.reserve(items.size());
    for (const auto& [id, item]: items) {
        result.push_back(item);
    }
    return result;
}

void GroundItems::restoreItems(const std::vector<LootData>& saved) {
    std::map<EntityId, LootData> restored;
    for (const LootData& item: saved) {
        if (item.id == 0 || item.quantity == 0 || !restored.emplace(item.id, item).second) {
            throw std::invalid_argument(server_constants::INVALID_SAVED_GROUND_ITEM);
        }
    }
    items = std::move(restored);
}
