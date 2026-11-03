#include "game.h"

#include <chrono>
#include <stdexcept>

#include "constants.h"
#include "foo.h"

Game::Game(NetworkManager& net, const std::string& player_name, const ServerStatus& initialStatus):
        player_name(player_name), net(net), ui(player_name, initialStatus.gameId) {
    currentState.update(initialStatus);
}

void Game::exe() {
    net.start();

    const auto rate = std::chrono::milliseconds(1000 / FPS);
    try {
        Foo::constant_rate_loop([this](long long it) { loopInternalLogic(it); }, rate, running);
    } catch (...) {
        net.quit();
        throw;
    }
    net.quit();
}

void Game::loopInternalLogic(long long it) {
    for (auto& cmd: ui.poll_input()) {
        net.push_to_send(cmd);
    }

    if (ui.quit_requested()) {
        running = false;
        return;
    }

    if (!net.isConnected()) {
        throw std::runtime_error("Se perdio la conexion con el servidor");
    }

    while (auto status = net.try_pop_recv()) {
        currentState.update(*status);
    }

    ui.update(currentState, it);
}
