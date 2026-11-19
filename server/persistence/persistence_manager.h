#pragma once

#include <filesystem>
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

    void createGame(const PersistenceDTO& game);
    PersistenceDTO loadGame(const std::string& gameId) const;
    void saveGame(const PersistenceDTO& game);

private:
    PersistenceReader reader;
    PersistenceWriter writer;
    std::vector<GameIndexDTO> games;

    void saveIndex() const;
};
