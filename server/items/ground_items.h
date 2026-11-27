#pragma once

#include <functional>
#include <map>
#include <vector>

#include "entities/entity_id_generator.h"

#include "dtos.h"

class PlayerManager;
class PlayerSnapshot;

class GroundItems {
private:
    std::map<EntityId, LootData> items;

    bool add(LootData data, EntityIdGenerator& ids);
    static int64_t squaredDistance(const Position& from, const Position& to);

public:
    static constexpr uint16_t PICKUP_RANGE = 24;

    bool drop(const Position& position, ItemId item, uint32_t quantity, Quality quality, EntityIdGenerator& ids);
    bool dropGold(const Position& position, uint32_t quantity, EntityIdGenerator& ids);
    bool collect(PlayerManager& players, uint16_t playerId, const std::function<bool(const Position&)>& hasClearPath);
    std::vector<LootData> getPublicData() const;
    void restoreItems(const std::vector<LootData>& saved);
};
