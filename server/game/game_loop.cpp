#include <algorithm>
#include <chrono>
#include <iostream>
#include <thread>

#include "time/game_clock.h"

#include "constants.h"
#include "game.h"

void Game::updateToolStates() {
    std::lock_guard<std::mutex> lock(mtx);
    playerManager.updateToolStates(std::chrono::steady_clock::now());
}

void Game::advancePlayers() {
    std::lock_guard<std::mutex> lock(mtx);
    if (community.finished())
        return;
    playerManager.advancePlayers(PLAYER_STEP);
}

void Game::sendStatus() {
    std::lock_guard<std::mutex> lock(mtx);
    playerManager.publishStatus(buildStatus());
    world.afterStatusPublished();
}

bool Game::advanceWorld(std::chrono::milliseconds elapsed) {
    std::lock_guard<std::mutex> lock(mtx);
    if (community.finished()) {
        updateCommunity();
        return false;
    }
    if (endDayRequested) {
        elapsed = std::max(elapsed, playerManager.remainingDayTime());
        endDayRequested = false;
    }
    const auto activeTime = playerManager.isDayEnded() ? std::chrono::milliseconds{0} : std::min(elapsed, playerManager.remainingDayTime());
    const auto remainingTime = playerManager.remainingDayTime() - activeTime;
    if (!playerManager.isDayEnded() && playerManager.hasConnectedPlayers()) {
        world.update(activeTime, playerManager);
    }
    updateCommunity();
    if (community.finished())
        return false;
    if (playerManager.advanceDay(elapsed)) {
        world.finishDay(playerManager, remainingTime);
        marketPrices.finishDay();
        merchants->finishDay();
        ++currentDay;
        ++saveRequests;
        updateCommunity();
        return true;
    }
    return false;
}

void Game::processCommands() {
    PlayerCommand command;
    while (should_keep_running() && input->try_pop(command)) {
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
            if (should_keep_running()) {
                sendStatus();
            }
            nextTick += rate;
            const auto now = std::chrono::steady_clock::now();
            if (nextTick < now) {
                const auto missedTicks = (now - nextTick) / rate;
                nextTick += rate * (missedTicks + 1);
            }
            std::this_thread::sleep_until(nextTick);
        }
    } catch (const ClosedQueue&) {
        // cola cerrada.
    } catch (const std::exception& e) {
        std::cerr << "Error in Game: " << e.what() << "\n";
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
        input->close();
    } catch (const std::runtime_error&) {
        // cola cerrada
    }
}

void Game::stop() {
    Thread::stop();

    try {
        input->close();
    } catch (const std::runtime_error&) {
        // Cola ya cerrada.
    }
}
