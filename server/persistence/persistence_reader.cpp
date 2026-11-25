#include "persistence_reader.h"

#include "server_constants.h"

#include <cctype>
#include <stdexcept>

#include "persistence_deserializer.h"

PersistenceReader::PersistenceReader(const std::filesystem::path& directory)
    : directory(directory) {}

void PersistenceReader::validateId(const std::string& gameId) {
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

std::filesystem::path PersistenceReader::gamePath(const std::string& gameId) const {
    validateId(gameId);
    return directory / (gameId + server_constants::FIELD_TOML);
}

toml::table PersistenceReader::readToml(const std::filesystem::path& path) {
    try {
        return toml::parse_file(path.string());
    } catch (const toml::parse_error& error) {
        throw std::runtime_error(server_constants::CANNOT_PARSE_TOML_FILE + path.string() +
                                 server_constants::QUOTED_VALUE_SEPARATOR + std::string(error.description()));
    }
}

std::vector<GameIndexDTO> PersistenceReader::readIndex() const {
    const std::filesystem::path path = directory / server_constants::INDEX_TOML;

    if (!std::filesystem::exists(path)) {
        return {};
    }

    toml::table data = readToml(path);
    const toml::array* games = data[server_constants::GAMES].as_array();
    if (games == nullptr) {
        throw std::runtime_error(server_constants::INVALID_INDEX_TOML_GAMES_MUST_BE_AN_ARRAY);
    }

    std::vector<GameIndexDTO> result;
    for (const auto& game : *games) {
        const toml::table* table = game.as_table();
        if (table == nullptr) {
            throw std::runtime_error(server_constants::INVALID_INDEX_TOML_EACH_GAME_MUST_BE_A_TABLE);
        }
        GameIndexDTO dto = PersistenceDeserializer::deserializeIndex(*table);
        validateId(dto.id);
        result.push_back(dto);
    }
    return result;
}

PersistenceDTO PersistenceReader::readGame(const std::string& gameId) const {
    const std::filesystem::path path = gamePath(gameId);
    if (!std::filesystem::exists(path)) {
        throw std::runtime_error(server_constants::GAME_FILE_NOT_FOUND_PREFIX + path.string());
    }

    PersistenceDTO dto = PersistenceDeserializer::deserializeGame(readToml(path));
    if (dto.id != gameId) {
        throw std::runtime_error(server_constants::GAME_ID_DOES_NOT_MATCH_FILENAME_PREFIX + path.string());
    }
    return dto;
}
