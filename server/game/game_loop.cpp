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
    playerManager.advancePlayers(PLAYER_STEP);
}

void Game::sendStatus() {
    std::lock_guard<std::mutex> lock(mtx);
    playerManager.publishStatus(buildStatus());
    world.afterStatusPublished();
}

void Game::advanceWorld(std::chrono::milliseconds elapsed) {
    std::lock_guard<std::mutex> lock(mtx);
    if (!playerManager.isDayEnded() && playerManager.hasConnectedPlayers()) {
        world.update(elapsed, playerManager.getCollisionBounds(Zone::Farm),playerManager.getCombatants());
    }
    if (world.combatCompleted()) {
        playerManager.applyCombatBonus();
    }
    if (playerManager.advanceDay(elapsed)) {
        world.finishDay();
    }
}

void Game::processCommands() {
    PlayerCommand command;
    while (should_keep_running() && input->try_pop(command)) {
        std::lock_guard<std::mutex> lock(mtx);
        playerManager.handleCommand(command);
    }
}

void Game::run() {
    const auto rate = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::duration<float, std::milli>{TICK_MS});
    GameClock clock;
    try {
        auto nextTick = std::chrono::steady_clock::now();
        while (should_keep_running()) {
            updateToolStates();
            processCommands();
            advancePlayers();
            advanceWorld(clock.advance());
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
        std::cerr << "Error en Game: " << e.what() << "\n";
    } catch (...) {
        std::cerr << "Error desconocido en Game\n";
    }

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
