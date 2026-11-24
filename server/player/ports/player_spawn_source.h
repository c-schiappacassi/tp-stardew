#pragma once
#include <cstdint>
#include <optional>

#include "world/world_location.h"
class PlayerSnapshot;
class PlayerSpawnSource {
public:
    virtual ~PlayerSpawnSource();
    virtual std::optional<WorldLocation> spawnFor(const PlayerSnapshot& player) const = 0;
    virtual std::optional<WorldLocation> recoveryFor(const PlayerSnapshot& player,uint16_t playerId) const;
};
