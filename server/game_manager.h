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


/*
class GameManager { // agus
public:
    GameManager(const GameManager&) = delete;
    GameManager& operator=(const GameManager&) = delete;

    GameSession createGame(const CreateGameDTO& dto,
                           const std::shared_ptr<OutQueue>& out);
    GameSession joinGame(const JoinGameDTO& dto,
                         const std::shared_ptr<OutQueue>& out);

    GameSession resumeGame(const ResumeGameDTO& dto,
                           const std::shared_ptr<OutQueue>& out);
    GameListDTO listGames();

};
*/