#pragma once

#include <chrono>
#include <optional>

#include "farm_targeting.h"

class EntityIdGenerator;
class FarmTerrain;
class CropManager;
class AnimalManager;
class DecorationManager;
class ChestManager;
class GroundItems;
class PlayerManager;
class PlayerSnapshot;

class FarmInteractions {
private:
    static std::optional<CropType> cropTypeForSeed(ItemId item);

    FarmTerrain& terrain;
    CropManager& crops;
    AnimalManager& animals;
    DecorationManager& decorations;
    ChestManager& chests;
    GroundItems& groundItems;
    FarmTargeting targeting;

    bool useShovel(const PlayerManager& players, uint16_t playerId, Direction direction, EntityIdGenerator& entityIds) const;
    bool waterCrop(const PlayerManager& players, uint16_t playerId, Direction direction) const;
    bool collectProduct(const PlayerManager& players, uint16_t playerId, Direction direction, EntityIdGenerator& entityIds) const;
    bool placeSelectedItem(PlayerManager& players, uint16_t playerId, EntityIdGenerator& entityIds, Direction direction, ItemId item) const;
    bool applySelectedItem(PlayerManager& players, uint16_t playerId, EntityIdGenerator& entityIds, Direction direction) const;

public:
    FarmInteractions(FarmTerrain& terrain, CropManager& crops, AnimalManager& animals, DecorationManager& decorations, ChestManager& chests, GroundItems& groundItems);
    bool collectNearbyItems(PlayerManager& players, uint16_t playerId) const;
    bool interact(PlayerManager& players, uint16_t playerId, EntityIdGenerator& entityIds, Direction direction, std::chrono::steady_clock::time_point now) const;
};
