#include "game_manager.h"

#include <algorithm>

#include <exception>
#include <optional>
#include <mutex>
#include <string>
#include <utility>
#include <stdexcept>


GameManager::GameManager(const GameConfig& config, const std::optional<FarmLayout>& farmLayout)
    : config(config),
      farmLayout(farmLayout),
      persistence("saves") {
    for (const GameIndexDTO& game : persistence.getGames()) {
        const std::string prefix = "game-";
        if (game.id.compare(0, prefix.size(), prefix) != 0) continue;
        try {
            std::size_t parsed = 0;
            unsigned long long number = std::stoull(game.id.substr(prefix.size()), &parsed);
            if (parsed == game.id.size() - prefix.size() && number >= nextId) {
                nextId = static_cast<std::size_t>(number + 1);
            }
        } catch (const std::exception&) {}
    }
}

std::unique_ptr<Game> GameManager::makeGame(
    const std::string& id, const std::string& name, uint8_t worldId) const {
    auto setup = farmLayout ? makeFarmGameSetup(worldId, *farmLayout) : makeDefaultGameSetup(worldId);
    if (config.town.map.width != 0) setup.townConfig = config.town;
    auto game = std::make_unique<Game>(id, name, worldId, config, setup.farm.playerHouses);
    if (!game->loadDungeon(setup.worldId, setup.layout)) {
        throw std::invalid_argument("Could not load the game dungeon");
    }

    if (!game->loadFarm(setup.worldId, setup.farm)) {
        throw std::invalid_argument("Could not load the game farm");
    }
    if (setup.townConfig.map.width != 0 && !game->loadTown(setup.townConfig)) {
        throw std::invalid_argument("The town or its connection to the farm is invalid");
    }
    if (setup.entrance && !game->configureDungeonEntrance(*setup.entrance)) {
        throw std::invalid_argument("The dungeon entrance is invalid");
    }
    return game;
}

GameSession GameManager::createGame(const CreateGameDTO& dto,
                                    const std::shared_ptr<OutQueue>& output) {
    std::lock_guard<std::mutex> lock(mtx);
    const std::string id = "game-" + std::to_string(nextId);
    auto game = makeGame(id, dto.gameName, dto.worldId);

    GameSession session = game->createGame(dto, output);

    if (session.status.status == StatusCode::Ok) {
        {
            std::lock_guard<std::mutex> saveLock(persistenceMtx);
            persistence.createGame(game->getPersistenceData());
        }
        game->setSaveCallback([this](const PersistenceDTO& data) {
            std::lock_guard<std::mutex> saveLock(persistenceMtx);
            persistence.saveGame(data);
        });
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
        session.status.errorMsg = "No game exists with that code";
        return session;
    }
    // The game loop owns subsequent saves, including new players and disconnects.
    return it->second->joinGame(dto, output);
}

GameSession GameManager::resumeGame(const ResumeGameDTO& dto,
                                    const std::shared_ptr<OutQueue>& output) {
    std::lock_guard<std::mutex> lock(mtx);
    auto it = games.find(dto.gameId);
    if (it != games.end()) return it->second->resumeGame(dto, output);

    GameSession failure{};
    failure.status.status = StatusCode::Error;
    failure.status.errorMsg = "Could not restore the game";
    try {
        PersistenceDTO saved;
        {
            std::lock_guard<std::mutex> saveLock(persistenceMtx);
            if (!persistence.gameExists(dto.gameId)) return failure;
            saved = persistence.loadGame(dto.gameId);
        }
        if (!saved.players.contains(dto.userName)) {
            failure.status.errorMsg = "You do not have a player in this game";
            return failure;
        }
        auto game = makeGame(saved.id, saved.farmName, saved.mapId);
        game->restorePersistenceData(saved);
        GameSession session = game->resumeGame(dto, output);
        if (session.status.status != StatusCode::Ok) return session;
        game->setSaveCallback([this](const PersistenceDTO& data) {
            std::lock_guard<std::mutex> saveLock(persistenceMtx);
            persistence.saveGame(data);
        });
        game->start();
        games.emplace(dto.gameId, std::move(game));
        return session;
    } catch (const std::exception& e) {
        failure.status.errorMsg = std::string("Error restoring the game: ") + e.what();
        return failure;
    }
}

bool GameManager::codeExists(const std::string& code) {
    std::lock_guard<std::mutex> lock(mtx);
    std::lock_guard<std::mutex> saveLock(persistenceMtx);
    return games.find(code) != games.end() || persistence.gameExists(code);
}

std::vector<GameInfoDTO> GameManager::listGames(const std::string& userName) {
    std::lock_guard<std::mutex> lock(mtx);
    std::lock_guard<std::mutex> saveLock(persistenceMtx);

    return persistence.listGamesFor(userName);
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
