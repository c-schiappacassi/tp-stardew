#ifndef GAME_MANAGER_H
#define GAME_MANAGER_H

#include <filesystem>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include "dtos.h"
#include "game.h"
#include "game_types.h"
#include "game_setup.h"
#include "../persistence/persistence_manager.h"

class GameManager {
private:
    std::mutex mtx;
    std::size_t nextId = 1;
    std::map<std::string, std::unique_ptr<Game>> games;
    GameConfig config;
    std::optional<FarmLayout> farmLayout;
    PersistenceManager persistence;
    std::mutex persistenceMtx;

    static std::filesystem::path resolveMapPath(const std::string& mapPath);
    std::unique_ptr<Game> makeGame(const std::string& id, const std::string& name,
                                   uint8_t worldId, const std::string& mapPath) const;

public:
    explicit GameManager(const GameConfig& config = {},
                         const std::optional<FarmLayout>& farmLayout = std::nullopt);
    ~GameManager();

    GameSession createGame(const CreateGameDTO& dto, const std::shared_ptr<OutQueue>& output);
    GameSession joinGame(const JoinGameDTO& dto, const std::shared_ptr<OutQueue>& output);
    GameSession resumeGame(const ResumeGameDTO& dto, const std::shared_ptr<OutQueue>& output);
    bool codeExists(const std::string& code);
    std::vector<GameInfoDTO> listGames(const std::string& userName);
    void stopAll();
};

#endif
