#include "game.h"

#include <chrono>
#include <thread>
#include <iostream>

#include "constants.h"
#include "foo.h"
#include "time/game_clock.h"

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
    world.update(elapsed, playerManager.getCollisionBounds(Zone::Farm));
}

void Game::processCommands() {
    PlayerCommand command;
    while (should_keep_running() && input->try_pop(command)) {
        std::lock_guard<std::mutex> lock(mtx);
        playerManager.handleCommand(command);
    }
}

void Game::run() {
    bool running = should_keep_running();
    const auto rate = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::duration<float, std::milli>{TICK_MS});
    GameClock clock;
    try {
        Foo::constant_rate_loop(
            [this, &running, &clock](long long) {
                updateToolStates();
                processCommands();
                advancePlayers();
                advanceWorld(clock.advance());
                running = should_keep_running();

                if (running) {
                    sendStatus();
                }
            },
            rate, running);
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
