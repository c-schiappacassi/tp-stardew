#ifndef GAME_MANAGER_H
#define GAME_MANAGER_H

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "dtos.h"
#include "game.h"
#include "game_types.h"

class GameManager {
private:
    std::mutex mtx;
    std::size_t nextId = 1;
    std::map<std::string, std::unique_ptr<Game>> games;

public:
    ~GameManager();

    GameSession createGame(const CreateGameDTO& dto, const std::shared_ptr<OutQueue>& output);
    GameSession joinGame(const JoinGameDTO& dto, const std::shared_ptr<OutQueue>& output);
    bool codeExists(const std::string& code);
    std::vector<GameInfoDTO> listGames(const std::string& userName);
    void stopAll();
};

#endif
