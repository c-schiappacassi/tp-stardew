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

class GameManager {
private:
    std::size_t nextId = 1;
    std::mutex mtx;
    std::vector<std::unique_ptr<Game>> games;

public:
    ~GameManager();

    GameSession createGame(const CreateGameDTO& dto,const std::shared_ptr<OutQueue>& out);
    GameSession joinGame(const JoinGameDTO& dto,const std::shared_ptr<OutQueue>& out);
    Game* findGame(const std::string& id);
};

#endif