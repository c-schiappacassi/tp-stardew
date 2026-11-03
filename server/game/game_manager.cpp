#include "game_manager.h"

#include <exception>
#include <optional>
#include <mutex>
#include <string>
#include <utility>
#include <stdexcept>


GameSession GameManager::createGame(const CreateGameDTO& dto,
                                    const std::shared_ptr<OutQueue>& output) {
    std::lock_guard<std::mutex> lock(mtx);
    const std::string id = "game-" + std::to_string(nextId);

    auto game = std::make_unique<Game>(id,"",setup.worldId,servicename,setup.playerConfig,setup.dungeonConfig);
    if (!game->loadDungeon(setup.worldId, setup.layout)) {
        throw std::invalid_argument("No se pudo cargar la mazmorra de la partida");
    }

    if (!game->configureDungeonEntrance(setup.entrance)) {
        throw std::invalid_argument("La entrada de la mazmorra es invalida");
    }

    games.push_back(std::move(game));

    if (session.status.status == StatusCode::Ok) {
        game->start();
        games.emplace(id, std::move(game));
        ++nextId;
    }
    return session;
}

GameSession GameManager::joinGame(const JoinGameDTO& dto,
                                  const std::shared_ptr<OutQueue>& output) {
    std::lock_guard<std::mutex> lock(mtx);

    auto it = games.find(dto.code);
    if (it == games.end()) {
        GameSession session{};
        session.status.status = StatusCode::Error;
        session.status.errorMsg = "No existe una partida con ese codigo";
        return session;
    }
    return it->second->joinGame(dto, output);
}

GameSession GameManager::resumeGame(const ResumeGameDTO& dto, const std::shared_ptr<OutQueue>& output) {
    std::lock_guard<std::mutex> lock(mtx);

    auto it = games.find(dto.gameId);
    if (it == games.end()) {
        GameSession session{};
        session.status.status = StatusCode::Error;
        session.status.errorMsg = "No existe esa partida";
        return session;
    }
    return it->second->resumeGame(dto, output);
}

bool GameManager::codeExists(const std::string& code) {
    std::lock_guard<std::mutex> lock(mtx);
    return games.find(code) != games.end();
}

std::vector<GameInfoDTO> GameManager::listGames(const std::string& userName) {
    std::lock_guard<std::mutex> lock(mtx);

    std::vector<GameInfoDTO> result;
    for (const auto& entry: games) {
        std::optional<GameInfoDTO> info = entry.second->infoFor(userName);
        if (info) {
            result.push_back(*info);
        }
    }
    return result;
}

void GameManager::stopAll() {
    std::lock_guard<std::mutex> lock(mtx);

    for (auto& entry: games) {
        entry.second->stop();
    }
    for (auto& entry: games) {
        entry.second->join();
    }
    games.clear();
}

GameManager::~GameManager() { stopAll(); }
