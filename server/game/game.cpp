#include "game.h"

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

Game::Game(const std::string& id, const std::string& name, uint8_t worldId, PreparedWorld prepared, const GameConfig& config, ChatServices chatServices):
    id(id), name(name), worldId(worldId),
    world(config.dungeon, std::random_device{}(), config.farm),
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
        return GameSessions::failure("The game has already been created");
    if (dto.gameName.empty())
        return GameSessions::failure("The game name is required");
    if (!world.acceptsWorld(dto.worldId))
        return GameSessions::failure("The loaded dungeon belongs to another world");

    name = dto.gameName;
    worldId = dto.worldId;
    auto session = sessions.createPlayer(dto, output);
    configured = session.status.status == StatusCode::Ok;
    return completeEntry(std::move(session));
}

GameSession Game::joinGame(const JoinGameDTO& dto, const std::shared_ptr<OutQueue>& output) {
    std::lock_guard<std::mutex> lock(mtx);
    if (!configured || dto.code != id || community.finished())
        return GameSessions::failure(community.finished() ? "The game has already ended" : "No game exists with that code");
    return completeEntry(sessions.joinPlayer(dto, output));
}

GameSession Game::resumeGame(const ResumeGameDTO& dto, const std::shared_ptr<OutQueue>& output) {
    std::lock_guard<std::mutex> lock(mtx);
    if (!configured || dto.gameId != id) {
        auto failure = GameSessions::failure("That game does not exist");
        failure.userName = dto.userName;
        return failure;
    }
    return completeEntry(sessions.resumePlayer(dto.userName, output));
}

GameSession Game::completeEntry(GameSession session) {
    if (session.status.status == StatusCode::Ok) {
        session.status.gameCompleted = community.finished();
        ++saveRequests;
    }
    return session;
}

CommandSubmission Game::enqueueCommand(uint16_t playerId, std::shared_ptr<CommandDTO> command) {
    try {
        input.push({playerId, std::move(command)});
        return CommandSubmission::Enqueued;
    } catch (const ClosedQueue&) {
        return CommandSubmission::GameClosed;
    }
}

void Game::setSaveCallback(std::function<void(const PersistenceDTO&)> callback) {
    std::lock_guard<std::mutex> lock(mtx);
    saveCallback = std::move(callback);
    ++saveRequests;
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
    snapshot.players = playerManager.getPersistencePlayers();
    snapshot.communityCenterActionProgress = CommunityPersistence::save(community, communityRewards);
    for (const auto& [userName, player]: snapshot.players)
        snapshot.users.push_back(userName);
    world.saveFarmAndStorage(snapshot);
    return snapshot;
}

void Game::restorePersistenceData(const PersistenceDTO& saved) {
    std::lock_guard<std::mutex> lock(mtx);
    if (configured || saved.id != id || saved.mapId != worldId || saved.currentDay == 0)
        throw std::runtime_error("Cannot restore game with mismatched identity or state");
    world.restoreFarm(saved);
    CommunityPersistence::restore(community, communityRewards, saved.communityCenterActionProgress);
    playerManager.restorePlayers(saved.players);
    name = saved.farmName;
    currentDay = saved.currentDay;
    configured = true;
    updateCommunity();
}

void Game::saveIfNeeded(bool force) {
    try {
        PersistenceDTO snapshot;
        std::function<void(const PersistenceDTO&)> callback;
        SaveRevision revision{};
        {
            std::lock_guard<std::mutex> lock(mtx);
            if (!configured || !saveCallback)
                return;
            revision = {saveRequests, community.getRevision(), communityRewards.getRevision()};
            if (!force && savedRevision && *savedRevision == revision)
                return;
            snapshot = buildPersistenceData();
            callback = saveCallback;
        }
        callback(snapshot);
        std::lock_guard<std::mutex> lock(mtx);
        savedRevision = revision;
    } catch (const std::exception& error) {
        std::cerr << "Error saving game " << id << ": " << error.what() << '\n';
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
    std::lock_guard<std::mutex> lock(mtx);
    if (!community.finished())
        playerManager.advancePlayers(PLAYER_STEP);
}

void Game::sendStatus() {
    std::lock_guard<std::mutex> lock(mtx);
    playerManager.publishStatus(buildStatus());
    world.afterStatusPublished();
}

void Game::advanceWorld(std::chrono::milliseconds elapsed) {
    std::lock_guard<std::mutex> lock(mtx);
    if (community.finished()) {
        updateCommunity();
        return;
    }
    if (endDayRequested) {
        elapsed = std::max(elapsed, playerManager.remainingDayTime());
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
        ++saveRequests;
        updateCommunity();
    }
}

void Game::processCommands() {
    PlayerCommand command;
    while (should_keep_running() && input.try_pop(command)) {
        std::lock_guard<std::mutex> lock(mtx);
        playerManager.handleCommand(command, community.finished());
        if (command.command && command.command->type() == MessageType::ExitGame)
            ++saveRequests;
        updateCommunity();
    }
}

void Game::updateCommunity() {
    community.observeCombat(world.getCombatProgress());
    communityRewards.updateMarket();
    if (community.finished() && !completionHandled) {
        playerManager.finishGame();
        completionHandled = true;
        ++saveRequests;
    }
    playerManager.collectCommunityRewards();
}

qtStatus Game::checkJoin(const CheckCodeDTO& dto) const {
    std::lock_guard<std::mutex> lock(mtx);

    qtStatus reply{};

    if (!configured || dto.code != id) {
        reply.status = StatusCode::Error;
        reply.errorMsg = "No game exists with that code";
    } else if (community.finished()) {
        reply.status = StatusCode::Error;
        reply.errorMsg = "The game has already ended";
    } else if (playerManager.dataFor(dto.userName).has_value()) {
        reply.status = StatusCode::Error;
        reply.errorMsg = "A player with that name already exists";
    } else if (!playerManager.hasAvailableSlot()) {
        reply.status = StatusCode::Error;
        reply.errorMsg = "The game is full";
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
            processCommands();
            advancePlayers();
            advanceWorld(clock.advance());
            saveIfNeeded();
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
        std::cerr << "Error in Game: " << error.what() << '\n';
    } catch (...) {
        std::cerr << "Unknown error in Game\n";
    }
    saveIfNeeded(true);
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
