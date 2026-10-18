#include "game.h"

Game::Game(const std::string& id, const std::string& name): id(id), name(name) {}

const std::string& Game::getId() const {
    return id;
}

const std::string& Game::getName() const {
    return name;
}