#pragma once

#include <vector>
#include "dtos.h"


// TODO
class State {
private:
    std::map<std::string, PlayerData> players;

public:
    State() = default;

    void update(const ServerStatus& status) {
        players = status.players;
    }

    const std::map<std::string, PlayerData>& get_players() const { return players; }

    const PlayerData* find_player(const std::string& name) const {
        auto it = players.find(name);
        return it == players.end() ? nullptr : &it->second;
    }
};