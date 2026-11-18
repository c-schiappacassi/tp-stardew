#include "game_manager.h"

#include <algorithm>

#include <exception>
#include <optional>
#include <mutex>
#include <string>
#include <utility>
#include <stdexcept>


GameManager::GameManager(const DungeonConfig& dungeonConfig,
                         const PlayerConfig& playerConfig,
                         const std::optional<FarmLayout>& farmLayout,
                         const TownConfig& townConfig,
                         const MarketConfig& marketConfig)
    : dungeonConfig(dungeonConfig),
      playerConfig(playerConfig),
      farmLayout(farmLayout),
      townConfig(townConfig),
      marketConfig(marketConfig) {
  
}

GameSession GameManager::createGame(const CreateGameDTO& dto,
                                    const std::shared_ptr<OutQueue>& output) {
    std::lock_guard<std::mutex> lock(mtx);
    const std::string id = "game-" + std::to_string(nextId);

    GameSetup setup = farmLayout ? makeFarmGameSetup(dto.worldId, *farmLayout) :makeDefaultGameSetup(dto.worldId);
    setup.dungeonConfig = dungeonConfig;
    setup.playerConfig = playerConfig;
    setup.farmConfig = FarmConfig{std::chrono::milliseconds{playerConfig.dayDurationMs}};
    if (townConfig.map.width != 0) setup.townConfig = townConfig;
    auto game = std::make_unique<Game>(id, dto.gameName, setup.worldId,
                                       setup.playerConfig, setup.dungeonConfig, setup.farmConfig,setup.farm.playerHouses,ChatServices{}, marketConfig);
    if (!game->loadDungeon(setup.worldId, setup.layout)) {
        throw std::invalid_argument("No se pudo cargar la mazmorra de la partida");
    }

    if (!game->loadFarm(setup.worldId, setup.farm)) {
        throw std::invalid_argument("No se pudo cargar la granja de la partida");
    }
    if (setup.townConfig.map.width != 0 && !game->loadTown(setup.townConfig)) {
        throw std::invalid_argument("El pueblo o su conexion con la granja es invalido");
    }
    if (setup.entrance && !game->configureDungeonEntrance(*setup.entrance)) {
        throw std::invalid_argument("La entrada de la mazmorra es invalida");
    }

    GameSession session = game->createGame(dto, output);

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
    GameSession session = it->second->joinGame(dto, output);

    return session;
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
