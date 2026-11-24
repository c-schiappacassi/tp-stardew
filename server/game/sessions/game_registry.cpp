#include "game_registry.h"

#include <exception>
#include <stdexcept>

#include "game/game.h"

GameRegistry::GameRegistry(const std::vector<GameIndexDTO>& saved) {
    const std::string prefix = "game-";
    for (const auto& game: saved) {
        if (game.id.compare(0, prefix.size(), prefix) != 0)
            continue;
        try {
            std::size_t parsed = 0;
            const auto number = std::stoull(game.id.substr(prefix.size()), &parsed);
            if (parsed == game.id.size() - prefix.size() && number >= nextId)
                nextId = static_cast<std::size_t>(number + 1);
        } catch (const std::exception&) {}
    }
}

std::string GameRegistry::nextGameId() const {
    std::lock_guard<std::mutex> lock(mtx);
    return "game-" + std::to_string(nextId);
}

std::shared_ptr<Game> GameRegistry::find(const std::string& id) const {
    std::lock_guard<std::mutex> lock(mtx);
    const auto it = games.find(id);
    return it == games.end() ? nullptr : it->second;
}

void GameRegistry::start(const std::string& id, const std::shared_ptr<Game>& game) {
    std::lock_guard<std::mutex> lock(mtx);
    const bool created = id == "game-" + std::to_string(nextId);
    const auto [it, inserted] = games.emplace(id, game);
    if (!inserted)
        throw std::logic_error("The game is already active");
    try {

        game->start();
    } catch (...) {
        games.erase(it);
        throw;
    }
    if (created)
        ++nextId;
}

void GameRegistry::stopAll() {
    std::map<std::string, std::shared_ptr<Game>> stopping;
    {
        std::lock_guard<std::mutex> lock(mtx);
        stopping.swap(games);
    }

    for (const auto& [id, game]: stopping)
        game->stop();
    for (const auto& [id, game]: stopping)
        game->join();
}
