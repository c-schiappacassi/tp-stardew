#include "game.h"

#include <chrono>
#include <stdexcept>

#include "constants.h"
#include "foo.h"

Game::Game(const std::string& host, const std::string& port, const std::string& player_name):
        player_name(player_name), net(host, port), ui(player_name) {}

void Game::connect_placeholder() {
    CreateGameDTO create(player_name);
    create.gameName = "test";
    create.worldId = 0;
    create.shirtModel = 0;
    create.hairColor = {163, 113, 5};
    create.pantsColor = {35, 39, 166};

    ServerStatus st = net.handshake(create);
    if (st.status != StatusCode::Ok) {
        throw std::runtime_error(st.errorMsg);
    }
    currentState.update(st);
}

void Game::exe() {
    connect_placeholder();

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

    while (auto status = net.try_pop_recv()) {
        currentState.update(*status);
    }

    ui.update(currentState, it);
}