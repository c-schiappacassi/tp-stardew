#include "game_manager.h"

#include <exception>
#include <mutex>
#include <string>
#include <utility>

GameSession GameManager::createGame(const CreateGameDTO& dto,const std::shared_ptr<OutQueue>& out) {
    if (dto.gameName.empty() || dto.playerName.empty()) {
        return {{StatusCode::Error,"El nombre de la partida y del jugador son obligatorios", ""},"", 0, 0, {}};
    }

    std::lock_guard<std::mutex> lock(mtx);

    const std::string id = "game-" + std::to_string(nextId);
    auto game = std::make_unique<Game>(id, dto.gameName, dto.worldId);

    uint16_t playerId = 0;
    if (!game->tryAddPlayer(dto.playerName, dto.skin, out, playerId)) {
        return {{StatusCode::Error, "No se pudo registrar al jugador", ""},"", 0, 0, {}};
    }

    auto input = game->getInputQueue();
    games.push_back(std::move(game));

    try {
        games.back()->start();
    } catch (const std::exception&) {
        games.pop_back();
        return {{StatusCode::Error, "No se pudo iniciar la partida", ""},"", 0, 0, {}};
    }

    ++nextId;

    return {{StatusCode::Ok, "", "Partida creada"},id, playerId, dto.worldId, input};
}

GameSession GameManager::joinGame(const JoinGameDTO& dto,const std::shared_ptr<OutQueue>& out) {
    if (dto.code.empty() || dto.playerName.empty()) {
        return {{StatusCode::Error,"El codigo y el nombre del jugador son obligatorios", ""},"", 0, 0, {}};
    }

    std::lock_guard<std::mutex> lock(mtx);

    for (const auto& game : games) {
        if (game->getId() != dto.code) {
            continue;
        }

        uint16_t playerId = 0;
        if (!game->tryAddPlayer(dto.playerName, dto.skin, out, playerId)) {
            return {{StatusCode::Error,"No se pudo unir el jugador a la partida", ""},"", 0, 0, {}};
        }

        return {{StatusCode::Ok, "", "Te uniste a la partida"},game->getId(), playerId, game->getWorldId(),game->getInputQueue()};
    }

    return {{StatusCode::Error,"No existe una partida con ese codigo", ""},"", 0, 0, {}};
}


Game* GameManager::findGame(const std::string& id) {
    std::lock_guard<std::mutex> lock(mtx);

    for (const auto& game : games) {
        if (game->getId() == id) {
            return game.get();
        }
    }

    return nullptr;
}

GameManager::~GameManager() {
    for (const auto& game : games) {
        game->stop();
    }

    for (const auto& game : games) {
        game->join();
    }
}