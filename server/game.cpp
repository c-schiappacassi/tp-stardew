#include "game.h"
#include <chrono>
#include <thread>
#include "constantes.h"
#include "foo.h"

Game::Game(const std::string& id, const std::string& name, uint8_t worldId, const char* servicename):
    id(id),name(name),worldId(worldId),input(std::make_shared<CommandQueue>()),skt(servicename),acceptor(skt, *this) {}

const std::string& Game::getId() const {
    return id;
}

std::string Game::getName() const {
    std::lock_guard<std::mutex> lock(mtx);
    return name;
}

uint8_t Game::getWorldId() const {
    std::lock_guard<std::mutex> lock(mtx);
    return worldId;
}

std::shared_ptr<CommandQueue> Game::getInputQueue() const {
    return input;
}

GameSession Game::createGame(const CreateGameDTO& dto,const std::shared_ptr<OutQueue>& output) {
    std::lock_guard<std::mutex> lock(mtx);
    GameSession session{};
    session.status.status = StatusCode::Error;

    if (configured) {
        session.status.errorMsg = "La partida ya fue creada";
        return session;
    }

    if (dto.gameName.empty()) {
        session.status.errorMsg = "El nombre de la partida es obligatorio";
        return session;
    }

    name = dto.gameName;
    worldId = dto.worldId;
    session = addPlayer(dto.playerName, dto.skin, output, "Partida creada");

    if (session.status.status == StatusCode::Ok) {
        configured = true;
    }

    return session;
}

GameSession Game::addPlayer(const std::string& playerName, uint8_t skin,const std::shared_ptr<OutQueue>& output,const std::string& successMsg) {
    GameSession session{};
    session.status.status = StatusCode::Error;

    if (playerName.empty() || !output) {
        session.status.errorMsg = "No se pudo registrar al jugador";
        return session;
    }

    if (players.size() >= 4) {
        session.status.errorMsg = "La partida esta llena";
        return session;
    }

    const uint16_t playerId = nextPlayerId;
    players.emplace(
            playerId, PlayerRecord{playerId, playerName, skin, output, true});
    ++nextPlayerId;

    session.status.status = StatusCode::Ok;
    session.status.successMsg = successMsg;
    session.status.gameId = id;
    session.status.playerId = playerId;
    session.status.worldId = worldId;
    session.gameId = id;
    session.playerId = playerId;
    session.worldId = worldId;
    session.input = input;

    return session;
}

GameSession Game::joinGame(const JoinGameDTO& dto,const std::shared_ptr<OutQueue>& output) {
    std::lock_guard<std::mutex> lock(mtx);

    if (!configured || dto.code != id) {
        GameSession session{};
        session.status.status = StatusCode::Error;
        session.status.errorMsg = "No existe una partida con ese codigo";
        return session;
    }

    return addPlayer(dto.playerName, dto.skin, output,"Te uniste a la partida");
}

void Game::disconnectPlayer(uint16_t playerId) {
    std::lock_guard<std::mutex> lock(mtx);

    auto it = players.find(playerId);
    if (it == players.end()) {
        return;
    }

    it->second.connected = false;
    it->second.output.reset();
}

void Game::movePlayer(uint16_t playerId, const MoveDTO& move) {
    std::lock_guard<std::mutex> lock(mtx);

    auto it = players.find(playerId);
    if (it == players.end() || !it->second.connected) {
        return;
    }

    it->second.position = move.position;
}

void Game::sendStatus() {
    std::lock_guard<std::mutex> lock(mtx);

    for (const auto& entry : players) {
        const PlayerRecord& player = entry.second;

        if (!player.connected) {
            continue;
        }

        ServerStatus status;
        status.status = StatusCode::Ok;
        status.positionPlayer = player.position;
        status.gameId = id;
        status.playerId = player.id;
        status.worldId = worldId;

        try {
            player.output->push(status);
        } catch (const ClosedQueue&) {
            // cliente desconectado y su salida ya fue cerrada.
        }
    }
}

void Game::processCommands() {
    PlayerCommand command;

    while (should_keep_running() && input->try_pop(command)) {
        if (std::holds_alternative<MoveDTO>(command.command)) {
            const MoveDTO& move = std::get<MoveDTO>(command.command);
            movePlayer(command.playerId, move);
        } else if (std::holds_alternative<DisconnectCommand>(command.command)) {
            disconnectPlayer(command.playerId);
        }
    }
}


void Game::run() {
    bool running = should_keep_running();
    const auto rate = std::chrono::milliseconds(1000 / FPS);

    acceptor.start();

    try {
        Foo::constant_rate_loop(
            [this, &running](long long) {
                processCommands();
                running = should_keep_running();

                if (running) {
                    sendStatus();
                }
            },
            rate, running);
    } catch (const ClosedQueue&) {
        // Cola cerrada al frenar la partida.
    } catch (...) {
        acceptor.stop();
        acceptor.join();
        throw;
    }

    acceptor.stop();
    acceptor.join();
}

void Game::stop() {
    Thread::stop();

    try {
        input->close();
    } catch (const std::runtime_error&) {
        // cola ya cerrada
    }
}