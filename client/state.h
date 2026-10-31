#pragma once

#include <vector>
#include "dtos.h"

// TODO
class State {
private:
    std::map<std::string, PlayerData> players;
    PlayerDataLocal local;
public:
    void update(const ServerStatus& s) { players = s.players; local = s.localPlayer; }
    const std::map<std::string, PlayerData>& get_players() const { return players; }
    const PlayerDataLocal& get_local() const { return local; }
    const PlayerData* find_player(const std::string& name) const {
        auto it = players.find(name);
        return it == players.end() ? nullptr : &it->second;
    }
};