#pragma once

class Player;

class PlayerSpawnSource {
public:
    virtual ~PlayerSpawnSource();
    virtual bool spawnPlayer(Player& player) const = 0;
    virtual bool recoverPlayer(Player& player) const { return spawnPlayer(player); }
};
