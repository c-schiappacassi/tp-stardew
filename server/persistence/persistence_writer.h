#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include <toml++/toml.h>

#include "persistence_dto.h"
#include "persistence_index.h"

class PersistenceWriter {
public:
    explicit PersistenceWriter(const std::filesystem::path& directory);

    void writeIndex(const std::vector<GameIndexDTO>& games) const;
    void writeGame(const PersistenceDTO& game) const;

private:
    std::filesystem::path directory;

    static void validateId(const std::string& gameId);
    static void writeToml(const std::filesystem::path& path, const toml::table& data);
    std::filesystem::path gamePath(const std::string& gameId) const;
};
