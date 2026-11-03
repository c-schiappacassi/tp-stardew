#pragma once

#include "player.h"
#include "player_config.h"

class PlayerFactory {
private:
    PlayerConfig config;

public:
    explicit PlayerFactory(const PlayerConfig& config);
    Player create(const PlayerData& appearance) const;
};