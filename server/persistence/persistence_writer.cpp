#include "persistence_writer.h"

#include <cctype>
#include <fstream>
#include <stdexcept>
#include <system_error>

#include "persistence_serializer.h"

PersistenceWriter::PersistenceWriter(const std::filesystem::path& directory)
    : directory(directory) {}

void PersistenceWriter::validateId(const std::string& gameId) {
    if (gameId.empty()) {
        throw std::invalid_argument("Game ID cannot be empty");
    }

    for (unsigned char character : gameId) {
        if (!std::isalnum(character) && character != '_' && character != '-') {
            throw std::invalid_argument("Invalid game ID: " + gameId);
        }
    }

    if (gameId == "index") {
        throw std::invalid_argument("The game ID 'index' is reserved");
    }
}

std::filesystem::path PersistenceWriter::gamePath(const std::string& gameId) const {
    validateId(gameId);
    return directory / (gameId + ".toml");
}

void PersistenceWriter::writeToml(const std::filesystem::path& path,
                                  const toml::table& data) {
    std::filesystem::create_directories(path.parent_path());

    std::filesystem::path temporary = path;
    temporary += ".tmp";

    try {
        {
            std::ofstream file(temporary, std::ios::out | std::ios::trunc);
            if (!file.is_open()) {
                throw std::runtime_error("Cannot open file for writing: " + temporary.string());
            }

            file << data;
            file.flush();
            if (!file.good()) {
                throw std::runtime_error("Cannot write TOML file: " + temporary.string());
            }
            file.close();
            if (file.fail()) {
                throw std::runtime_error("Cannot close TOML file: " + temporary.string());
            }
        }

        std::filesystem::rename(temporary, path);
    } catch (...) {
        std::error_code error;
        std::filesystem::remove(temporary, error);
        throw;
    }
}

void PersistenceWriter::writeIndex(const std::vector<GameIndexDTO>& games) const {
    toml::array entries;
    for (const GameIndexDTO& game : games) {
        validateId(game.id);
        entries.push_back(PersistenceSerializer::serialize(game));
    }

    toml::table data;
    data.insert("games", std::move(entries));
    writeToml(directory / "index.toml", data);
}

void PersistenceWriter::writeGame(const PersistenceDTO& game) const {
    const std::filesystem::path path = gamePath(game.id);
    writeToml(path, PersistenceSerializer::serialize(game));
}
