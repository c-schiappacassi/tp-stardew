#ifndef GAME_MANAGER_H
#define GAME_MANAGER_H

#include <cstddef>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "dtos.h"
#include "game.h"
#include "game_types.h"
#include "game_setup.h"

class GameManager {
private:
    std::size_t nextId = 1;
    std::mutex mtx;
    std::vector<std::unique_ptr<Game>> games;

public:
    ~GameManager();

    std::string createGame(const char* servicename,const GameSetup& setup = {});
    void stopAll();
    Game* findGame(const std::string& id);
};

#endif