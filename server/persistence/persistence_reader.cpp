#include "persistence_reader.h"

#include <cctype>
#include <stdexcept>

#include "persistence_deserializer.h"

PersistenceReader::PersistenceReader(const std::filesystem::path& directory)
    : directory(directory) {}

void PersistenceReader::validateId(const std::string& gameId) {
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

std::filesystem::path PersistenceReader::gamePath(const std::string& gameId) const {
    validateId(gameId);
    return directory / (gameId + ".toml");
}

toml::table PersistenceReader::readToml(const std::filesystem::path& path) {
    try {
        return toml::parse_file(path.string());
    } catch (const toml::parse_error& error) {
        throw std::runtime_error("Cannot parse TOML file '" + path.string() +
                                 "': " + std::string(error.description()));
    }
}

std::vector<GameIndexDTO> PersistenceReader::readIndex() const {
    const std::filesystem::path path = directory / "index.toml";

    if (!std::filesystem::exists(path)) {
        return {};
    }

    toml::table data = readToml(path);
    const toml::array* games = data["games"].as_array();
    if (games == nullptr) {
        throw std::runtime_error("Invalid index.toml: 'games' must be an array");
    }

    std::vector<GameIndexDTO> result;
    for (const auto& game : *games) {
        const toml::table* table = game.as_table();
        if (table == nullptr) {
            throw std::runtime_error("Invalid index.toml: each game must be a table");
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
        throw std::runtime_error("Game file not found: " + path.string());
    }

    PersistenceDTO dto = PersistenceDeserializer::deserializeGame(readToml(path));
    if (dto.id != gameId) {
        throw std::runtime_error("Game ID does not match filename: " + path.string());
    }
    return dto;
}
