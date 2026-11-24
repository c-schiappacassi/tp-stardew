#include "player_spawn_source.h"
PlayerSpawnSource::~PlayerSpawnSource() = default;
std::optional<WorldLocation> PlayerSpawnSource::recoveryFor(const PlayerSnapshot& player,uint16_t) const {
    return spawnFor(player);
}
