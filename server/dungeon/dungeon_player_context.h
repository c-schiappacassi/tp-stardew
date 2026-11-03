#pragma once

#include <cstdint>
#include <optional>
#include "dungeon/dungeon.h"
#include "dungeon/dungeon_entrance.h"
#include "player/ports/movement_rules.h"
#include "player/ports/zone_data_source.h"
#include "player/ports/player_interactions.h"

class DungeonPlayerContext final: public MovementRules,public ZoneDataSource,public PlayerInteractions {
private:
    Dungeon& dungeon;
    const std::optional<DungeonEntrance>& entrance;
    const std::optional<uint8_t>& loadedWorld;
    bool usePassage(Player& player);

public:
    DungeonPlayerContext(Dungeon& dungeon,const std::optional<DungeonEntrance>& entrance,const std::optional<uint8_t>& loadedWorld);
    bool canMove(const Player& player, const Position& next) const override;
    void completeZoneData(const Player& player, PlayerDataLocal& data,bool includeBroken) const override;
    void interact(Player& player, const InteractDTO& interaction) override;
};