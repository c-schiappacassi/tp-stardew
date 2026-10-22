#include "game.h"

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
    players.push_back(PlayerRecord{id, playerName, skin, output, true});
    assignedId = id;
    ++nextPlayerId;
    return true;
}

void Game::disconnectPlayer(uint16_t playerId) {
    std::lock_guard<std::mutex> lock(mtx);

    for (PlayerRecord& player : players) {
        if (player.id == playerId) {
            player.connected = false;
            player.output.reset();
            return;
        }
    }
}

void Game::run() {
    try {
        while (should_keep_running()) {
            const PlayerCommand command = input->pop();

            if (std::holds_alternative<DisconnectCommand>(command.command)) {
                disconnectPlayer(command.playerId);
            }
        }
    } catch (const ClosedQueue&) {
        // cola cerrada al frenar la partidaa
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