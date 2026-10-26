#pragma once

#include <vector>
#include "dtos.h"


// TODO
class State {
private:
    std::vector<PlayerData> players;

public:
    State() = default;

    void update(const ServerStatus&) {

    }
};
