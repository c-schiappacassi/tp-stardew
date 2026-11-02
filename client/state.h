#pragma once

#include <vector>
#include "dtos.h"

// TODO
class State {
private:
    std::map<uint8_t, std::vector<MineralData>> dungeon_minerals;
    std::map<std::string, PlayerData> players;
    PlayerDataLocal local;
public:
    void update(const ServerStatus& s) { players = s.players; local = s.localPlayer; dungeon_minerals = s.dungeonMinerals; }
    const std::map<std::string, PlayerData>& get_players() const { return players; }
    const PlayerDataLocal& get_local() const { return local; }
    const std::map<uint8_t, std::vector<MineralData>>& get_dungeon_minerals() const { return dungeon_minerals; }
    const PlayerData* find_player(const std::string& name) const {
        auto it = players.find(name);
        return it == players.end() ? nullptr : &it->second;
    }
};