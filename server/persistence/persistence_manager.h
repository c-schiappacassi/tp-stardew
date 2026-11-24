#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "persistence_dto.h"
#include "persistence_index.h"
#include "persistence_reader.h"
#include "persistence_writer.h"

class PersistenceManager {
public:
    explicit PersistenceManager(const std::filesystem::path& directory);

    std::vector<GameIndexDTO> getGames() const;
    bool gameExists(const std::string& gameId) const;
    std::string mapPathFor(const std::string& gameId) const;
    std::vector<GameInfoDTO> listGamesFor(const std::string& userName) const;
    void registerGame(const GameIndexDTO& game);
    void updateGameIndex(const GameIndexDTO& game);

    void createGame(const PersistenceDTO& game, const std::string& mapPath);
    PersistenceDTO loadGame(const std::string& gameId) const;
    void saveGame(const PersistenceDTO& game);

private:
    PersistenceReader reader;
    PersistenceWriter writer;
    std::vector<GameIndexDTO> games;

    void saveIndex() const;
    static GameIndexDTO indexFromGame(const PersistenceDTO& game);
};
