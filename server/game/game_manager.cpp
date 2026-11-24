#include "game_manager.h"

#include <exception>
#include <utility>

#include "game.h"
#include "loading/game_loader.h"
#include "sessions/game_sessions.h"

GameManager::GameManager(const GameConfig& config, const std::optional<FarmLayout>&):
    config(config), persistence("saves"), games(persistence.getGames()) {}

GameManager::~GameManager() { stopAll(); }

void GameManager::startGame(const std::string& id, const std::shared_ptr<Game>& game) {
    game->setSaveCallback([this](const PersistenceDTO& data) {
        std::lock_guard<std::mutex> lock(persistenceMtx);
        persistence.saveGame(data);
    });
    games.start(id, game);
}

GameSession GameManager::createGame(const CreateGameDTO& dto, const std::shared_ptr<OutQueue>& output) {
    std::lock_guard<std::mutex> lock(lifecycleMtx);
    try {
        const auto id = games.nextGameId();
        auto prepared = GameLoader::prepare(dto.mapPath, dto.worldId);
        auto game = std::make_shared<Game>(id, dto.gameName, dto.worldId, std::move(prepared), config);
        auto session = game->createGame(dto, output);
        if (session.status.status != StatusCode::Ok)
            return session;
        {
            std::lock_guard<std::mutex> saveLock(persistenceMtx);
            persistence.createGame(game->getPersistenceData(), dto.mapPath);
        }
        startGame(id, game);
        return session;
    } catch (const std::exception& error) {
        return GameSessions::failure(std::string("Could not create game: ") + error.what());
    }
}

GameSession GameManager::joinGame(const JoinGameDTO& dto, const std::shared_ptr<OutQueue>& output) {
    std::lock_guard<std::mutex> lock(lifecycleMtx);
    const auto game = games.find(dto.code);
    return game ? game->joinGame(dto, output) : GameSessions::failure("No game exists with that code");
}

GameSession GameManager::resumeGame(const ResumeGameDTO& dto, const std::shared_ptr<OutQueue>& output) {
    std::lock_guard<std::mutex> lock(lifecycleMtx);
    if (const auto game = games.find(dto.gameId))
        return game->resumeGame(dto, output);
    try {
        PersistenceDTO saved;
        std::string mapPath;
        {
            std::lock_guard<std::mutex> saveLock(persistenceMtx);
            if (!persistence.gameExists(dto.gameId))
                return GameSessions::failure("Could not restore the game");
            saved = persistence.loadGame(dto.gameId);
            mapPath = persistence.mapPathFor(dto.gameId);
        }
        if (!saved.players.contains(dto.userName))
            return GameSessions::failure("You do not have a player in this game");
        auto prepared = GameLoader::prepare(mapPath, saved.mapId);
        auto game = std::make_shared<Game>(saved.id, saved.farmName, saved.mapId, std::move(prepared), config);
        game->restorePersistenceData(saved);
        auto session = game->resumeGame(dto, output);
        if (session.status.status != StatusCode::Ok)
            return session;
        startGame(dto.gameId, game);
        return session;
    } catch (const std::exception& error) {
        return GameSessions::failure(std::string("Error restoring the game: ") + error.what());
    }
}

bool GameManager::codeExists(const std::string& code) {
    std::lock_guard<std::mutex> lock(lifecycleMtx);
    std::lock_guard<std::mutex> saveLock(persistenceMtx);
    return games.find(code) != nullptr || persistence.gameExists(code);
}

std::vector<GameInfoDTO> GameManager::listGames(const std::string& userName) {
    std::lock_guard<std::mutex> lock(lifecycleMtx);
    std::lock_guard<std::mutex> saveLock(persistenceMtx);
    return persistence.listGamesFor(userName);
}

CommandSubmission GameManager::enqueueCommand(const std::string& gameId, uint16_t playerId, std::shared_ptr<CommandDTO> command) {
    const auto game = games.find(gameId);
    return game ? game->enqueueCommand(playerId, std::move(command)) : CommandSubmission::GameNotFound;
}

qtStatus GameManager::checkJoin(const CheckCodeDTO& dto) {
    std::lock_guard<std::mutex> lock(lifecycleMtx);
    const auto game = games.find(dto.code);
    qtStatus reply{};
    if (game) {
        reply = game->checkJoin(dto);
    } else {
        reply.status = StatusCode::Error;
        reply.errorMsg = "No game exists with that code";
    }
    return reply;
}

void GameManager::stopAll() {
    std::lock_guard<std::mutex> lock(lifecycleMtx);
    games.stopAll();
}
