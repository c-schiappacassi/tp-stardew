#pragma once

#include <cstdint>

class Player;

class PlayerSpawnSource {
public:
    virtual ~PlayerSpawnSource();
    virtual bool spawnPlayer(Player& player) const = 0;
    virtual bool recoverPlayer(Player& player, uint16_t = 0) const { return spawnPlayer(player); }
};
