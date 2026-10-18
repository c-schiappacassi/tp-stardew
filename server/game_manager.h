#ifndef GAME_MANAGER_H
#define GAME_MANAGER_H

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include "game.h"

class GameManager {
public:
    Game& createGame(const std::string& name);
    Game* findGame(const std::string& id);

private:
    std::size_t nextId = 1;
    std::vector<std::unique_ptr<Game>> games;
};

#endif