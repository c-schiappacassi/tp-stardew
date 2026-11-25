#include "game_loader.h"

#include "server_constants.h"

#include <stdexcept>

#include "common/map/map_loader.h"
#include "world/world_manager.h"

std::filesystem::path GameLoader::resolveMapPath(const std::string& mapPath) {
    const std::filesystem::path relative(mapPath);
    if (relative.empty() || relative.is_absolute() || relative.extension() != server_constants::FIELD_TOML)
        throw std::invalid_argument(server_constants::INVALID_MAP_PATH_PREFIX + mapPath);

    auto part = relative.begin();
    if (part == relative.end() || *part++ != server_constants::MAPS || part == relative.end() || (*part != server_constants::PREDEFINED && *part != server_constants::CUSTOM))
        throw std::invalid_argument(server_constants::MAP_MUST_BE_IN_MAPS_PREDEFINED_OR_MAPS_CUSTOM);
    for (const auto& component: relative) {
        if (component == server_constants::DOT || component == server_constants::PARENT_DIRECTORY)
            throw std::invalid_argument(server_constants::MAP_PATH_CANNOT_CONTAIN_TRAVERSAL_COMPONENTS);
    }

    auto directory = std::filesystem::current_path();
    while (true) {
        const auto candidate = directory / relative;
        if (std::filesystem::is_regular_file(candidate)) {
            const auto canonicalRoot = std::filesystem::weakly_canonical(directory / server_constants::MAPS);
            const auto canonicalFile = std::filesystem::canonical(candidate);
            const auto inside = canonicalFile.lexically_relative(canonicalRoot);
            if (inside.empty() || inside.is_absolute() || *inside.begin() == server_constants::PARENT_DIRECTORY)
                throw std::invalid_argument(server_constants::MAP_FILE_IS_OUTSIDE_THE_MAPS_DIRECTORY);
            return canonicalFile;
        }
        if (directory == directory.root_path())
            break;
        directory = directory.parent_path();
    }
    throw std::runtime_error(server_constants::MAP_TOML_NOT_FOUND_PREFIX + mapPath);
}

PreparedWorld GameLoader::prepare(const std::string& mapPath, uint8_t worldId,
                                  const InteriorTemplates& interiors) {
    const auto data = MapLoader{}.load(resolveMapPath(mapPath));
    return WorldManager::prepareMap(data, worldId, interiors);
}

void GameLoader::initialize(WorldManager& world, const WorldSetup& setup) {
    if (!world.loadDungeon(setup.worldId, setup.layout))
        throw std::invalid_argument(server_constants::COULD_NOT_LOAD_THE_GAME_DUNGEON);
    if (!world.loadFarm(setup.worldId, setup.farm))
        throw std::invalid_argument(server_constants::COULD_NOT_LOAD_THE_GAME_FARM);
    if (setup.townConfig.map.width != 0 && !world.loadTown(setup.townConfig))
        throw std::invalid_argument(server_constants::THE_TOWN_OR_ITS_CONNECTION_TO_THE_FARM_IS_INVALID);
    if (setup.entrance && !world.configureDungeonEntrance(*setup.entrance))
        throw std::invalid_argument(server_constants::THE_DUNGEON_ENTRANCE_IS_INVALID);
}
