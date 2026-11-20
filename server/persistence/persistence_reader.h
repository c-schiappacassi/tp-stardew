#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include <toml++/toml.h>

#include "persistence_dto.h"
#include "persistence_index.h"

class PersistenceReader {
public:
    explicit PersistenceReader(const std::filesystem::path& directory);

    std::vector<GameIndexDTO> readIndex() const;
    PersistenceDTO readGame(const std::string& gameId) const;

private:
    std::filesystem::path directory;

    static void validateId(const std::string& gameId);
    static toml::table readToml(const std::filesystem::path& path);
    std::filesystem::path gamePath(const std::string& gameId) const;
};
