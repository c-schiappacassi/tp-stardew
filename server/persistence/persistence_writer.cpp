#include "persistence_writer.h"

#include "server_constants.h"

#include <cctype>
#include <fstream>
#include <stdexcept>
#include <system_error>

#include "persistence_serializer.h"

PersistenceWriter::PersistenceWriter(const std::filesystem::path& directory)
    : directory(directory) {}

void PersistenceWriter::validateId(const std::string& gameId) {
    if (gameId.empty()) {
        throw std::invalid_argument(server_constants::GAME_ID_CANNOT_BE_EMPTY);
    }

    for (unsigned char character : gameId) {
        if (!std::isalnum(character) && character != '_' && character != '-') {
            throw std::invalid_argument(server_constants::INVALID_GAME_ID_PREFIX + gameId);
        }
    }

    if (gameId == server_constants::INDEX) {
        throw std::invalid_argument(server_constants::THE_GAME_ID_INDEX_IS_RESERVED);
    }
}

std::filesystem::path PersistenceWriter::gamePath(const std::string& gameId) const {
    validateId(gameId);
    return directory / (gameId + server_constants::FIELD_TOML);
}

void PersistenceWriter::writeToml(const std::filesystem::path& path,
                                  const toml::table& data) {
    std::filesystem::create_directories(path.parent_path());

    std::filesystem::path temporary = path;
    temporary += server_constants::FIELD_TMP;

    try {
        {
            std::ofstream file(temporary, std::ios::out | std::ios::trunc);
            if (!file.is_open()) {
                throw std::runtime_error(server_constants::CANNOT_OPEN_FILE_FOR_WRITING_PREFIX + temporary.string());
            }

            file << data;
            file.flush();
            if (!file.good()) {
                throw std::runtime_error(server_constants::CANNOT_WRITE_TOML_FILE_PREFIX + temporary.string());
            }
            file.close();
            if (file.fail()) {
                throw std::runtime_error(server_constants::CANNOT_CLOSE_TOML_FILE_PREFIX + temporary.string());
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
    data.insert(server_constants::GAMES, std::move(entries));
    writeToml(directory / server_constants::INDEX_TOML, data);
}

void PersistenceWriter::writeGame(const PersistenceDTO& game) const {
    const std::filesystem::path path = gamePath(game.id);
    writeToml(path, PersistenceSerializer::serialize(game));
}
