#include "game.h"
#include <chrono>
#include <thread>
#include "constantes.h"
#include "foo.h"

Game::Game(const std::string& id, const std::string& name, uint8_t worldId): id(id),name(name),worldId(worldId),input(std::make_shared<CommandQueue>()) {}

const std::string& Game::getId() const {
    return id;
}

const std::string& Game::getName() const {
    return name;
}

uint8_t Game::getWorldId() const {
    return worldId;
}

std::shared_ptr<CommandQueue> Game::getInputQueue() const {
    return input;
}

bool Game::tryAddPlayer(const std::string& playerName,uint8_t skin,const std::shared_ptr<OutQueue>& output,uint16_t& assignedId) {
    if (playerName.empty() || !output) {
        return false;
    }

    std::lock_guard<std::mutex> lock(mtx);

    if (players.size() >= 4) {
        return false;
    }

    const uint16_t id = nextPlayerId;
    players.emplace(id, PlayerRecord{id, playerName, skin, output, true});
    assignedId = id;
    ++nextPlayerId;
    return true;
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

    try {
        Foo::constant_rate_loop(
            [this, &running](long long) {
                processCommands();
                running = should_keep_running();
                if (running) {
                    sendStatuses();
                }
            },
            rate, running);
    } catch (const ClosedQueue&) {
        // cola cerrada al frenar la partida
    }
}

void Game::stop() {
    Thread::stop();

    try {
        input->close();
    } catch (const std::runtime_error&) {
        // cola ya cerrada
    }
}