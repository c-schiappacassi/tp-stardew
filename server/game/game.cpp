#include "game.h"

#include "server_constants.h"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <thread>
#include <utility>

#include "chat/cheat_chat.h"
#include "community/chat/community_chat.h"
#include "constants.h"
#include "loading/game_loader.h"
#include "persistence/community_persistence.h"
#include "time/game_clock.h"

Game::Game(const std::string& id, const std::string& name, uint8_t worldId, PreparedWorld prepared, const GameConfig& config, ChatServices chatServices, std::optional<uint32_t> savedSeed):
    id(id), name(name), worldId(worldId),
    input(server_constants::COMMAND_QUEUE_SIZE),
    worldSeed(savedSeed ? *savedSeed : std::random_device{}()),
    world(config.dungeon, worldSeed, config.farm),
    commerce(std::make_shared<CommerceManager>(world, config.market)),
    community(config.community), communityRewards(community, world, *commerce),
    communityChat(std::make_shared<CommunityChat>(community, communityRewards)),
    playerManager(config.player, world, world, world, world, prepared.setup.farm.playerHouses,
        ChatServices{chatServices.merchants ? std::move(chatServices.merchants) : commerce,
            chatServices.cheats ? std::move(chatServices.cheats) : std::make_shared<CheatChat>(world, [this] { endDayRequested = true; }),
            chatServices.community ? std::move(chatServices.community) : communityChat}),
    sessions(this->id, this->worldId, playerManager, world, std::move(prepared.visuals)) {
    GameLoader::initialize(world, prepared.setup);
}

GameSession Game::createGame(const CreateGameDTO& dto, const std::shared_ptr<OutQueue>& output) {
    std::lock_guard<std::mutex> lock(mtx);
    if (configured)
        return GameSessions::failure(server_constants::THE_GAME_HAS_ALREADY_BEEN_CREATED);
    if (dto.gameName.empty())
        return GameSessions::failure(server_constants::THE_GAME_NAME_IS_REQUIRED);
    if (!world.acceptsWorld(dto.worldId))
        return GameSessions::failure(server_constants::THE_LOADED_DUNGEON_BELONGS_TO_ANOTHER_WORLD);

    name = dto.gameName;
    worldId = dto.worldId;
    auto session = sessions.createPlayer(dto, output);
    configured = session.status.status == StatusCode::Ok;
    return session;
}

GameSession Game::joinGame(const JoinGameDTO& dto, const std::shared_ptr<OutQueue>& output, const PlayerRegistration& registerPlayer) {
    std::lock_guard<std::mutex> lock(mtx);
    if (!configured || dto.code != id || community.finished())
        return GameSessions::failure(community.finished() ? server_constants::THE_GAME_HAS_ALREADY_ENDED : server_constants::NO_GAME_EXISTS_WITH_THAT_CODE);
    return sessions.joinPlayer(dto, output, registerPlayer);
}

GameSession Game::resumeGame(const ResumeGameDTO& dto, const std::shared_ptr<OutQueue>& output) {
    std::lock_guard<std::mutex> lock(mtx);
    if (!configured || dto.gameId != id) {
        auto failure = GameSessions::failure(server_constants::THAT_GAME_DOES_NOT_EXIST);
        failure.userName = dto.userName;
        return failure;
    }
    auto session = sessions.resumePlayer(dto.userName, output);
    if (session.status.status == StatusCode::Ok)
        session.status.gameCompleted = community.finished();
    return session;
}

CommandSubmission Game::enqueueCommand(uint16_t playerId, std::shared_ptr<CommandDTO> command) {
    try {
        PlayerCommand entry{playerId, std::move(command)};
        while (!input.try_push(entry))
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        return CommandSubmission::Enqueued;
    } catch (const ClosedQueue&) {
        return CommandSubmission::GameClosed;
    }
}

void Game::setSaveCallback(std::function<void(const PersistenceDTO&)> callback) {
    std::lock_guard<std::mutex> lock(mtx);
    saveCallback = std::move(callback);
}

PersistenceDTO Game::getPersistenceData() const {
    std::lock_guard<std::mutex> lock(mtx);
    return buildPersistenceData();
}

PersistenceDTO Game::buildPersistenceData() const {
    PersistenceDTO snapshot;
    snapshot.id = id;
    snapshot.farmName = name;
    snapshot.mapId = worldId;
    snapshot.currentDay = currentDay;
    snapshot.worldSeed = worldSeed;
    snapshot.players = playerManager.getPersistencePlayers();
    snapshot.communityCenterActionProgress = CommunityPersistence::save(community, communityRewards);
    snapshot.users = playerManager.playerNames();
    world.saveFarmAndStorage(snapshot);
    return snapshot;
}

void Game::restorePersistenceData(const PersistenceDTO& saved) {
    std::lock_guard<std::mutex> lock(mtx);
    if (configured || saved.id != id || saved.mapId != worldId || saved.currentDay == 0)
        throw std::runtime_error(server_constants::CANNOT_RESTORE_GAME_WITH_MISMATCHED_IDENTITY_OR_STATE);
    world.restoreFarm(saved);
    CommunityPersistence::restore(community, communityRewards, saved.communityCenterActionProgress);
    playerManager.restorePlayers(saved.players, saved.users);
    name = saved.farmName;
    currentDay = saved.currentDay;
    configured = true;
    completionHandled = community.finished();
    updateCommunity();
}

void Game::savePendingDay() {
    try {
        std::shared_ptr<const PersistenceDTO> snapshot;
        std::function<void(const PersistenceDTO&)> callback;
        {
            std::lock_guard<std::mutex> lock(mtx);
            if (!pendingSave || !saveCallback)
                return;
            snapshot = pendingSave;
            callback = saveCallback;
        }
        callback(*snapshot);
        std::lock_guard<std::mutex> lock(mtx);
        if (pendingSave == snapshot)
            pendingSave.reset();
    } catch (const std::exception& error) {
        std::cerr << server_constants::ERROR_SAVING_GAME_PREFIX << id << server_constants::COLON_SPACE << error.what() << '\n';
    }
}

ServerStatus Game::buildStatus() const {
    ServerStatus status{};
    status.status = StatusCode::Ok;
    status.gameId = id;
    status.worldId = worldId;
    status.gameCompleted = community.finished();
    return status;
}

void Game::updateToolStates() {
    std::lock_guard<std::mutex> lock(mtx);
    playerManager.updateToolStates(std::chrono::steady_clock::now());
}

void Game::advancePlayers() {
    if (!community.finished())
        playerManager.advancePlayers(PLAYER_STEP);
}

void Game::sendStatus() {
    std::lock_guard<std::mutex> lock(mtx);
    playerManager.publishStatus(buildStatus());
    world.afterStatusPublished();
}

void Game::advanceWorld(std::chrono::milliseconds elapsed) {
    if (community.finished()) {
        updateCommunity();
        return;
    }
    if (endDayRequested) {
        elapsed = playerManager.remainingDayTime();
        endDayRequested = false;
    }
    const auto activeTime = playerManager.isDayEnded() ? std::chrono::milliseconds{0} : std::min(elapsed, playerManager.remainingDayTime());
    const auto remainingTime = playerManager.remainingDayTime() - activeTime;
    if (!playerManager.isDayEnded() && playerManager.hasConnectedPlayers())
        world.update(activeTime, playerManager);
    updateCommunity();
    if (community.finished())
        return;
    if (playerManager.advanceDay(elapsed)) {
        world.finishDay(playerManager, remainingTime);
        commerce->finishDay();
        ++currentDay;
        updateCommunity();
        pendingSave = std::make_shared<PersistenceDTO>(buildPersistenceData());
    }
}

void Game::processCommands() {
    PlayerCommand command;
    unsigned int processed = 0;
    try {
        while (should_keep_running() && processed++ < server_constants::COMMAND_QUEUE_SIZE &&
               input.try_pop(command)) {
            playerManager.handleCommand(command, community.finished());
            updateCommunity();
        }
    } catch (const ClosedQueue&) {
        // Game cerro
    }
}

void Game::updateCommunity() {
    community.observeCombat(world.getCombatProgress());
    communityRewards.updateMarket();
    playerManager.collectCommunityRewards();
    if (community.finished() && !completionHandled) {
        playerManager.finishGame();
        completionHandled = true;
        pendingSave = std::make_shared<PersistenceDTO>(buildPersistenceData());
    }
}

bool Game::disconnectAndCheckEmpty(uint16_t playerId) {
    std::lock_guard<std::mutex> lock(mtx);
    // Apply earlier commands (including the last DayOk) before handling the exit.
    processCommands();
    advanceWorld(std::chrono::milliseconds{0});
    playerManager.disconnectPlayer(playerId);
    return !playerManager.hasConnectedPlayers();
}

qtStatus Game::checkJoin(const CheckCodeDTO& dto) const {
    std::lock_guard<std::mutex> lock(mtx);

    qtStatus reply{};

    if (!configured || dto.code != id) {
        reply.status = StatusCode::Error;
        reply.errorMsg = server_constants::NO_GAME_EXISTS_WITH_THAT_CODE;
    } else if (community.finished()) {
        reply.status = StatusCode::Error;
        reply.errorMsg = server_constants::THE_GAME_HAS_ALREADY_ENDED;
    } else if (playerManager.dataFor(dto.userName).has_value()) {
        reply.status = StatusCode::Error;
        reply.errorMsg = server_constants::A_PLAYER_WITH_THAT_NAME_ALREADY_EXISTS;
    } else if (!playerManager.hasAvailableSlot()) {
        reply.status = StatusCode::Error;
        reply.errorMsg = server_constants::THE_GAME_IS_FULL;
    } else {
        reply.status = StatusCode::Ok;
    }

    return reply;
}

void Game::run() {
    const auto rate = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::duration<float, std::milli>{TICK_MS});
    GameClock clock;
    try {
        auto nextTick = std::chrono::steady_clock::now();
        while (should_keep_running()) {
            updateToolStates();
            {
                std::lock_guard<std::mutex> lock(mtx);
                processCommands();
                advancePlayers();
                advanceWorld(clock.advance());
            }
            savePendingDay();
            if (should_keep_running())
                sendStatus();
            nextTick += rate;
            const auto now = std::chrono::steady_clock::now();
            if (nextTick < now) {
                const auto missedTicks = (now - nextTick) / rate;
                nextTick += rate * (missedTicks + 1);
            }
            std::this_thread::sleep_until(nextTick);
        }
    } catch (const ClosedQueue&) {
        // La entrada se cerro durante la detencion de la partida.
    } catch (const std::exception& error) {
        std::cerr << server_constants::ERROR_IN_GAME_PREFIX << error.what() << '\n';
    } catch (...) {
        std::cerr << server_constants::UNKNOWN_ERROR_IN_GAME_PREFIX;
    }
    savePendingDay();
    closeAll();
}

void Game::closeAll() {
    {
        std::lock_guard<std::mutex> lock(mtx);
        playerManager.closeOutputs();
    }
    try {
        input.close();
    } catch (const std::runtime_error&) {
        // stop() puede haber cerrado la entrada.
    }
}

void Game::stop() {
    Thread::stop();
    try {
        input.close();
    } catch (const std::runtime_error&) {
        // La partida ya estaba cerrada.
    }
}
