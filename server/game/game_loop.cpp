#include "game.h"

#include <chrono>
#include <thread>

#include "constants.h"
#include "foo.h"

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

void Game::processCommands() {
    PlayerCommand command;
    while (should_keep_running() && input->try_pop(command)) {
        std::lock_guard<std::mutex> lock(mtx);
        playerManager.handleCommand(command);
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
        // Cola ya cerrada.
    }
}