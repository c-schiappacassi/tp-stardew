#include "game.h"

#include "constants.h"


void Game::exe() {
    // TODO: este es un placeholder hasta que este qt, va  a volar
    CreateGameDTO create(player_name);
    create.gameName = "test";
    create.worldId = 0;
    create.shirtModel = 1;

    ServerStatus st = net.handshake(create);
    if (st.status != StatusCode::Ok) {
        throw std::runtime_error(st.errorMsg);
    }
    currentState.update(st);

    net.start();
    auto rate = std::chrono::milliseconds(1000 / FPS);
    Foo::constant_rate_loop([this](long long it) { loopInternalLogic(it); }, rate, running);
    net.quit();
}

void Game::loopInternalLogic(long long it) {
    for (auto& cmd : translator.poll()) {
        net.push_to_send(cmd);
    }
    if (translator.quit_requested()) {
        running = false;
        return;
    }

    while (auto status = net.try_pop_recv()) {
        currentState.update(*status);
    }
    ui.update(currentState, it);
}