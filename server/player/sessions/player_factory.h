#pragma once

#include "player/player.h"
#include "player/player_config.h"

class PlayerFactory {
private:
    PlayerConfig config;

public:
    explicit PlayerFactory(const PlayerConfig& config);
    Player create(const PlayerData& appearance) const;
    Player restore(const PlayerDataLocal& saved) const;
};
