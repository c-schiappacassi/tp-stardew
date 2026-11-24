#pragma once

#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include "config/game_config.h"
#include "game_types.h"
#include "persistence/persistence_manager.h"
#include "sessions/game_registry.h"

class Game;

class GameManager {
private:
    std::mutex lifecycleMtx;
    GameConfig config;
    PersistenceManager persistence;
    std::mutex persistenceMtx;
    GameRegistry games;

    void startGame(const std::string& id, const std::shared_ptr<Game>& game);

public:
    explicit GameManager(const GameConfig& config = {}, const std::optional<FarmLayout>& farmLayout = std::nullopt);
    ~GameManager();

    GameManager(const GameManager&) = delete;
    GameManager& operator=(const GameManager&) = delete;
    GameManager(GameManager&&) = delete;
    GameManager& operator=(GameManager&&) = delete;

    GameSession createGame(const CreateGameDTO& dto, const std::shared_ptr<OutQueue>& output);
    GameSession joinGame(const JoinGameDTO& dto, const std::shared_ptr<OutQueue>& output);
    GameSession resumeGame(const ResumeGameDTO& dto, const std::shared_ptr<OutQueue>& output);
    bool codeExists(const std::string& code);
    std::vector<GameInfoDTO> listGames(const std::string& userName);
    CommandSubmission enqueueCommand(const std::string& gameId, uint16_t playerId, std::shared_ptr<CommandDTO> command);
    void stopAll();
};
