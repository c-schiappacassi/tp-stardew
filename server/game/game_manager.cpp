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
      marketConfig(marketConfig),
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
        session.status.errorMsg = "No existe una partida con ese codigo";
        return session;
    }
    GameSession session = it->second->joinGame(dto, output);
    if (session.status.status == StatusCode::Ok) {
        {
            std::lock_guard<std::mutex> saveLock(persistenceMtx);
            persistence.saveGame(it->second->getPersistenceData());
        }
    }
    return session;
}

GameSession GameManager::resumeGame(const ResumeGameDTO& dto,
                                    const std::shared_ptr<OutQueue>& output) {
    std::lock_guard<std::mutex> lock(mtx);
    auto it = games.find(dto.gameId);
    if (it != games.end()) return it->second->resumeGame(dto, output);

    GameSession failure{};
    failure.status.status = StatusCode::Error;
    failure.status.errorMsg = "No se pudo recuperar la partida";
    if (!persistence.gameExists(dto.gameId)) return failure;

    try {
        PersistenceDTO saved = persistence.loadGame(dto.gameId);
        if (!saved.players.contains(dto.userName)) {
            failure.status.errorMsg = "No tenes un jugador en esta partida";
            return failure;
        }
        GameSetup setup = farmLayout ? makeFarmGameSetup(saved.mapId, *farmLayout) :
                                      makeDefaultGameSetup(saved.mapId);
        setup.dungeonConfig = dungeonConfig;
        setup.playerConfig = playerConfig;
        setup.farmConfig = FarmConfig{std::chrono::milliseconds{playerConfig.dayDurationMs}};
        if (townConfig.map.width != 0) setup.townConfig = townConfig;
        auto game = std::make_unique<Game>(saved.id, saved.farmName, saved.mapId,
            setup.playerConfig, setup.dungeonConfig, setup.farmConfig,
            setup.farm.playerHouses, ChatServices{}, marketConfig);
        if (!game->loadDungeon(setup.worldId, setup.layout) ||
            !game->loadFarm(setup.worldId, setup.farm) ||
            (setup.townConfig.map.width != 0 && !game->loadTown(setup.townConfig)) ||
            (setup.entrance && !game->configureDungeonEntrance(*setup.entrance))) {
            throw std::runtime_error("Saved game map could not be initialized");
        }
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
        failure.status.errorMsg = std::string("Error al recuperar la partida: ") + e.what();
        return failure;
    }
}

bool GameManager::codeExists(const std::string& code) {
    std::lock_guard<std::mutex> lock(mtx);
    return games.find(code) != games.end() || persistence.gameExists(code);
}

std::vector<GameInfoDTO> GameManager::listGames(const std::string& userName) {
    std::lock_guard<std::mutex> lock(mtx);

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
