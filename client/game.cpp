#include "game.h"

void Game::exe() {
    auto rate = std::chrono::milliseconds(1000/FPS);

    Foo::constant_rate_loop([this](long long it) {
        this->loopInternalLogic(it);
    }, rate, this->running);
}

void Game::loopInternalLogic(long long it){
    auto dto_opt = this->translator.try_pop();
    if (dto_opt.has_value()) {
        this->net.push_to_send(dto_opt.value());
    }

    auto serverStatus_opt = this->net.try_pop_recv();
    if (serverStatus_opt.has_value()) {
        this->currentState.update(serverStatus_opt.value());
    }
    ui.update(currentState, it);
}
