#pragma once

#include <cstddef>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "persistence/persistence_index.h"

class Game;

class GameRegistry {
private:
    mutable std::mutex mtx;
    std::size_t nextId = 1;
    std::map<std::string, std::shared_ptr<Game>> games;

public:
    explicit GameRegistry(const std::vector<GameIndexDTO>& saved);

    GameRegistry(const GameRegistry&) = delete;
    GameRegistry& operator=(const GameRegistry&) = delete;
    GameRegistry(GameRegistry&&) = delete;
    GameRegistry& operator=(GameRegistry&&) = delete;

    std::string nextGameId() const;
    std::shared_ptr<Game> find(const std::string& id) const;
    void start(const std::string& id, const std::shared_ptr<Game>& game);
    void stopAll();
};
