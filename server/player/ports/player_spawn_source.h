#pragma once

class Player;

class PlayerSpawnSource {
public:
    virtual ~PlayerSpawnSource();
    virtual bool spawnPlayer(Player& player) const = 0;
};
