#pragma once

#include <chrono>
#include <map>
#include <optional>
#include <vector>

#include "chests/chest_manager.h"
#include "entities/collision_box.h"
#include "entities/placement/placement_space.h"
#include "farm/decorations/decoration_manager.h"
#include "items/ground_items.h"

#include "interior_layout.h"

class PlayerManager;
class PlayerSnapshot;

class Interior {
private:
    static const CollisionBox BED_BOX;
    static constexpr int OFFSET_Y = -22;

    static bool validPosition(const Position& position);
    static std::vector<CollisionBounds> staticFurnitureBounds(const BackgroundData& background);

    InteriorLocation location;
    WorldLocation outside;
    std::vector<Position> extraOutside;
    Position returnPosition;
    Position entry;
    Position exit;
    std::vector<Position> beds;
    BackgroundData background;
    std::vector<CollisionBounds> furnitureObstacles;
    PlacementSpace space;
    ChestManager chests;
    DecorationManager decorations;
    GroundItems groundItems;

    bool canApproachBed(const Position& position, const Position& bed) const;
    ZoneMap walkingMap() const;
    std::vector<CollisionBounds> fixedObstacles() const;
    bool blockedByFixedObstacle(const CollisionBounds& bounds) const;
    std::optional<bool> interactWithDecoration(PlayerManager& players, uint16_t playerId, Direction direction, EntityIdGenerator& ids, std::chrono::steady_clock::time_point now);

public:
    explicit Interior(const InteriorLayout& layout);
    static bool validLayout(const InteriorLayout& layout);
    bool hasAccessibleBeds() const;
    bool initializeStorage(const std::vector<ChestSpawn>& initial, EntityIdGenerator& ids);
    const InteriorLocation& getLocation() const;
    const WorldLocation& getOutside() const;
    const std::vector<Position>& getExtraOutside() const;
    const Position& getReturnPosition() const;
    const Position& getEntry() const;
    bool hasBeds() const;
    bool sameBaseCell(const Position& first, const Position& second) const;
    bool canWalk(const CollisionBounds& bounds) const;
    bool canMove(const PlayerSnapshot& player, const Position& next) const;
    bool useBed(PlayerManager& players, uint16_t playerId) const;
    std::optional<WorldLocation> recoveryFor(const PlayerSnapshot& player) const;
    std::optional<NpcData> nearbyMerchant(const PlayerSnapshot& player, Direction direction) const;
    BackgroundData backgroundData() const;
    InteriorData data() const;
    bool useSelectedItem(PlayerManager& players, uint16_t playerId, Direction direction, EntityIdGenerator& ids, std::chrono::steady_clock::time_point now);
    std::optional<OpenChestData> openChestFor(const PlayerSnapshot& player) const;
    void depositInChest(PlayerManager& players, uint16_t playerId, const ItemTransferDTO& command);
    void withdrawFromChest(PlayerManager& players, uint16_t playerId, const SelectItemDTO& command);
    void collectNearbyItems(PlayerManager& players, uint16_t playerId);
    void saveInventories(std::map<uint32_t, std::map<uint8_t, Stack>>& inventories) const;
    void restoreStorage(const InteriorData& saved, const std::map<uint32_t, std::map<uint8_t, Stack>>& inventories);
};
