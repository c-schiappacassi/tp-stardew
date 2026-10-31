#include "game.h"
#include <chrono>
#include <thread>
#include "constants.h"
#include "foo.h"
#include <random>
#include <utility>

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

    if (dungeonWorldId.has_value() && *dungeonWorldId != dto.worldId) {
        session.status.errorMsg ="La mazmorra cargada corresponde a otro mundo";
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
    spawn.zone = Zone::Farm;
    spawn.dungeonLevel = 0;

    std::mt19937 generator(std::random_device{}());
    std::uniform_int_distribution<uint32_t> goldDistribution(0, 100);

    Player character(spawn,PLAYER_INVENTORY_CAPACITY,PLAYER_INITIAL_HEALTH,PLAYER_INITIAL_ENERGY,goldDistribution(generator));

    for (const ItemId tool : {ItemId::Pickaxe,ItemId::WateringCan,ItemId::Shovel,ItemId::Sword}) {
        character.addItem(Stack{tool, 1, Quality::None});
    }

    auto it = players.emplace(playerId, PlayerRecord{playerId,playerName,output,true,std::move(character)}).first;
    ++nextPlayerId;

    session.status = buildStatus();
    session.status.playerId = playerId;
    session.status.localPlayer = it->second.player.getLocalData();
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

    PlayerData data;
    data.hairModel = dto.hairModel;
    data.shirtModel = dto.shirtModel;
    data.pantsModel = dto.pantsModel;
    data.hairColor = dto.hairColor;
    data.pantsColor = dto.pantsColor;

    return addPlayer(dto.playerName,data,output,"Te uniste a la partida");
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

void Game::advancePlayers() {
    std::lock_guard<std::mutex> lock(mtx);

    for (auto& [id, record] : players) {
        if (!record.connected) {
            continue;
        }

        Player& player = record.player;

        if (player.getZone() != Zone::Dungeon) {
            player.advance(PLAYER_STEP);
            continue;
        }

        const DungeonRoom* room =dungeon.findRoom(player.getDungeonLevel());

        if (room == nullptr) {
            continue;
        }

        for (int distance = 0; distance < PLAYER_STEP; ++distance) {
            const Position next = player.getNextPosition(1);

            if (!room->canWalk(next)) {
                break;
            }

            player.advance(1);
        }
    }
}

void Game::movePlayer(uint16_t playerId, const MoveDTO& move) {
    std::lock_guard<std::mutex> lock(mtx);

    auto it = players.find(playerId);
    if (it == players.end() || !it->second.connected) {
        return;
    }

    it->second.player.setMovement(move.dir);
}

bool Game::mineMineral(uint16_t playerId, EntityId mineralId) {
    std::lock_guard<std::mutex> lock(mtx);

    auto it = players.find(playerId);
    if (it == players.end() || !it->second.connected) {
        return false;
    }

    return dungeon.mineMineral(it->second.player, mineralId);
}

void Game::updateToolStates() {
    std::lock_guard<std::mutex> lock(mtx);
    const auto now = std::chrono::steady_clock::now();

    for (auto& [id, record] : players) {
        if (record.connected) {
            record.player.updateToolState(now);
        }
    }
}

bool Game::setDungeonRoomMap(uint8_t roomNumber, const ZoneMap& map) {
    std::lock_guard<std::mutex> lock(mtx);
    if (configured) {
        return false;
    }

    DungeonRoom* room = dungeon.findRoom(roomNumber);
    if (room == nullptr) {
        return false;
    }

    return room->setMap(map);
}

bool Game::loadDungeon(uint8_t selectedWorldId,const std::map<uint8_t, DungeonRoomLayout>& layout) {
    std::lock_guard<std::mutex> lock(mtx);

    if (configured) {
        return false;
    }

    if (!dungeon.initialize(layout, entityIds)) {
        return false;
    }

    dungeonWorldId = selectedWorldId;
    return true;
}

ServerStatus Game::buildStatus(bool includeBroken) const {
    ServerStatus status;
    status.status = StatusCode::Ok;
    status.gameId = id;
    status.worldId = worldId;
    status.dungeonMinerals = dungeon.getMineralData(includeBroken);

    for (const auto& [id, record] : players) {
        if (record.connected) {
            status.players.emplace(
                record.name,
                record.player.getPublicData());
        }
    }
    return status;
}

void Game::sendStatus() {
    std::lock_guard<std::mutex> lock(mtx);

    ServerStatus status = buildStatus(true);

    for (const auto& [id, record] : players) {
        if (!record.connected) {
            continue;
        }

        status.playerId = record.id;
        status.localPlayer = record.player.getLocalData();

        try {
            record.output->push(status);
        } catch (const ClosedQueue&) {
            // cliente desconectado y su salida ya fue cerrada.
        }
    }
    //una vez enviados los minerales rotos los elimanmos
    dungeon.removeBrokenMinerals();
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

    acceptor.start();

    try {
        Foo::constant_rate_loop(
            [this, &running](long long) {
                updateToolStates();
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
