#include "persistence_manager.h"

#include "server_constants.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

GameIndexPlayerDTO PersistenceManager::previewFor(const PlayerDataLocal& player) {
    GameIndexPlayerDTO preview;
    preview.characterName = player.characterName;
    preview.hairModel = player.hairModel;
    preview.shirtModel = player.shirtModel;
    preview.pantsModel = player.pantsModel;
    preview.hairColor = player.hairColor;
    preview.pantsColor = player.pantsColor;
    return preview;
}

PersistenceManager::PersistenceManager(const std::filesystem::path& directory)
    : reader(directory), writer(directory), games(reader.readIndex()) {
    for (size_t i = 0; i < games.size(); ++i) {
        for (size_t j = i + 1; j < games.size(); ++j) {
            if (games[i].id == games[j].id) {
                throw std::runtime_error(server_constants::DUPLICATE_GAME_ID_IN_PERSISTENCE_INDEX_PREFIX + games[i].id);
            }
        }
    }
}

std::vector<GameIndexDTO> PersistenceManager::getGames() const {
    return games;
}

bool PersistenceManager::gameExists(const std::string& gameId) const {
    for (const GameIndexDTO& game : games) {
        if (game.id == gameId) {
            return true;
        }
    }
    return false;
}

std::string PersistenceManager::mapPathFor(const std::string& gameId) const {
    for (const GameIndexDTO& entry : games) {
        if (entry.id == gameId) return entry.mapPath;
    }
    throw std::runtime_error(server_constants::GAME_IS_NOT_REGISTERED_PREFIX + gameId);
}

void PersistenceManager::saveIndex() const {
    writer.writeIndex(games);
}


GameIndexDTO PersistenceManager::indexFromGame(const PersistenceDTO& game) {
    GameIndexDTO entry;
    entry.id = game.id;
    entry.farmName = game.farmName;
    entry.users = game.users;
    entry.isMultiplayer = game.users.size() > 1;
    for (const auto& [userName, player] : game.players) {
        entry.playerPreviews.emplace(userName, previewFor(player));
    }
    return entry;
}

std::vector<GameInfoDTO> PersistenceManager::listGamesFor(const std::string& userName) const {
    std::vector<GameInfoDTO> result;
    for (const GameIndexDTO& game : games) {
        if (std::find(game.users.begin(), game.users.end(), userName) == game.users.end()) {
            continue;
        }
        auto it = game.playerPreviews.find(userName);
        if (it == game.playerPreviews.end()) {
            throw std::runtime_error(server_constants::MISSING_PLAYER_PREVIEW_FOR_PREFIX + userName);
        }
        PlayerData player{};
        player.characterName = it->second.characterName;
        player.hairModel = it->second.hairModel;
        player.shirtModel = it->second.shirtModel;
        player.pantsModel = it->second.pantsModel;
        player.hairColor = it->second.hairColor;
        player.pantsColor = it->second.pantsColor;
        result.push_back(GameInfoDTO{game.id, game.farmName, player, game.isMultiplayer});
    }
    return result;
}

void PersistenceManager::registerPlayer(const std::string& gameId, const std::string& userName,
                                        const PlayerDataLocal& initial) {
    for (const auto& game : games) {
        if (game.id != gameId)
            continue;
        if (game.playerPreviews.contains(userName))
            throw std::runtime_error(server_constants::A_PLAYER_WITH_THAT_NAME_ALREADY_EXISTS);
        auto updated = game;
        auto preview = previewFor(initial);
        preview.initialPlayer = initial;
        updated.playerPreviews.emplace(userName, std::move(preview));
        updated.users.push_back(userName);
        updated.isMultiplayer = updated.users.size() > 1;
        updateGameIndex(updated);
        return;
    }
    throw std::runtime_error(server_constants::GAME_IS_NOT_REGISTERED_PREFIX + gameId);
}

void PersistenceManager::registerGame(const GameIndexDTO& game) {
    if (gameExists(game.id)) {
        throw std::runtime_error(server_constants::GAME_ALREADY_EXISTS_PREFIX + game.id);
    }
    games.push_back(game);
    try {
        saveIndex();
    } catch (...) {
        games.pop_back();
        throw;
    }
}

void PersistenceManager::updateGameIndex(const GameIndexDTO& game) {
    for (GameIndexDTO& entry : games) {
        if (entry.id == game.id) {
            GameIndexDTO previous = entry;
            entry = game;
            try {
                saveIndex();
            } catch (...) {
                entry = previous;
                throw;
            }
            return;
        }
    }
    throw std::runtime_error(server_constants::GAME_IS_NOT_REGISTERED_PREFIX + game.id);
}

void PersistenceManager::createGame(const PersistenceDTO& game, const std::string& mapPath) {
    if (gameExists(game.id)) {
        throw std::runtime_error(server_constants::GAME_ALREADY_EXISTS_PREFIX + game.id);
    }

    if (mapPath.empty()) {
        throw std::invalid_argument(server_constants::GAME_MAP_PATH_IS_REQUIRED);
    }
    GameIndexDTO entry = indexFromGame(game);
    entry.mapPath = mapPath;

    writer.writeGame(game);

    games.push_back(entry);
    try {
        saveIndex();
    } catch (...) {
        games.pop_back();
        throw;
    }
}

PersistenceDTO PersistenceManager::loadGame(const std::string& gameId) const {
    for (const auto& entry : games) {
        if (entry.id != gameId)
            continue;
        auto saved = reader.readGame(gameId);
        for (const auto& name : entry.users) {
            const auto& preview = entry.playerPreviews.at(name);
            if (preview.initialPlayer && saved.players.try_emplace(name, *preview.initialPlayer).second)
                saved.users.push_back(name);
        }
        return saved;
    }
    throw std::runtime_error(server_constants::GAME_IS_NOT_REGISTERED_PREFIX + gameId);
}

void PersistenceManager::saveGame(const PersistenceDTO& game) {
    for (GameIndexDTO& entry : games) {
        if (entry.id == game.id) {
            writer.writeGame(game);
            GameIndexDTO updated = indexFromGame(game);
            updated.mapPath = entry.mapPath;
            // A player can register after the end-of-day snapshot was taken.
            for (const auto& name : entry.users) {
                const auto& preview = entry.playerPreviews.at(name);
                if (preview.initialPlayer && updated.playerPreviews.emplace(name, preview).second)
                    updated.users.push_back(name);
            }
            updated.isMultiplayer = updated.users.size() > 1;
            updateGameIndex(updated);
            return;
        }
    }
    throw std::runtime_error(server_constants::CANNOT_SAVE_UNREGISTERED_GAME_PREFIX + game.id);
}
