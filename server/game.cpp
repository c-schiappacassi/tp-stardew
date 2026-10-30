#include "game.h"
#include <chrono>
#include <thread>
#include "constants.h"
#include "foo.h"


Game::Game(const std::string& id):
        id(id), name(""), worldId(0), input(std::make_shared<CommandQueue>()) {}


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
    PlayerData data;
    data.hairModel = dto.hairModel;
    data.shirtModel = dto.shirtModel;
    data.pantsModel = dto.pantsModel;
    data.hairColor = dto.hairColor;
    data.pantsColor = dto.pantsColor;

    session = addPlayer(dto.playerName, data, output, "Partida creada");

    if (session.status.status == StatusCode::Ok) {
        configured = true;
    }

    return session;
}

void Game::advancePlayers() {
    std::lock_guard<std::mutex> lock(mtx);

    for (auto& entry : players) {
        PlayerRecord& player = entry.second;
        if (!player.connected || player.data.state != PlayerState::Walking) {
            continue;
        }

        int x = player.data.pos.getX();
        int y = player.data.pos.getY();
        switch (player.data.dir) {
            case Direction::Up: y -= PLAYER_STEP; break;
            case Direction::Down: y += PLAYER_STEP; break;
            case Direction::Left: x -= PLAYER_STEP; break;
            case Direction::Right: x += PLAYER_STEP; break;
            case Direction::None: break;
        }
        player.data.pos = Position(std::max(0, x), std::max(0, y));
    }
}

GameSession Game::addPlayer(const std::string& playerName,const PlayerData& data,const std::shared_ptr<OutQueue>& output,const std::string& successMsg) {
    GameSession session{};
    session.status.status = StatusCode::Error;
    session.playerName = playerName;
    if (playerName.empty() || !output) {
        session.status.errorMsg = "No se pudo registrar al jugador";
        return session;
    }

    for (const auto& entry : players) {
        if (entry.second.name == playerName) {
            session.status.errorMsg = "Ya existe un jugador con ese nombre";
            return session;
        }
    }

    if (players.size() >= 4) {
        session.status.errorMsg = "La partida esta llena";
        return session;
    }

    const uint16_t playerId = nextPlayerId;
    PlayerData spawn = data;
    spawn.pos = Position(SPAWN_X, SPAWN_Y);
    players.emplace(playerId, PlayerRecord{playerId, playerName, output, true, spawn});
    ++nextPlayerId;

    session.status = buildStatus();
    session.status.playerId = playerId;
    session.status.successMsg = successMsg;
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

    return addPlayer(dto.playerName, PlayerData{}, output, "Te uniste a la partida");
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

    PlayerData& data = it->second.data;

    if (move.dir == Direction::None) {
        data.state = PlayerState::Idle;
    } else {
        data.dir = move.dir;
        data.state = PlayerState::Walking;
    }
}

ServerStatus Game::buildStatus() const {
    ServerStatus status;
    status.status = StatusCode::Ok;
    status.gameId = id;
    status.worldId = worldId;

    for (const auto& entry : players) {
        const PlayerRecord& player = entry.second;

        if (player.connected) {
            status.players.emplace(player.name, player.data);
        }
    }

    return status;
}

void Game::sendStatus() {
    std::lock_guard<std::mutex> lock(mtx);

    ServerStatus status = buildStatus();

    for (const auto& entry : players) {
        const PlayerRecord& player = entry.second;

        if (!player.connected) {
            continue;
        }

        status.playerId = player.id;

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
        if (!command.command) {
            continue;
        }
        if (command.command->type() == MessageType::Move) {
            const MoveDTO& move = static_cast<const MoveDTO&>(*command.command);
            movePlayer(command.playerId, move);
        } else if (command.command->type() == MessageType::ExitGame) {
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
                advancePlayers();
                running = should_keep_running();

                if (running) {
                    sendStatus();
                }
            },
            rate, running);
    } catch (const ClosedQueue&) {
        // Cola cerrada al frenar la partida.
}

void Game::stop() {
    Thread::stop();

    try {
        input->close();
    } catch (const std::runtime_error&) {
        // cola ya cerrada
    }
}
