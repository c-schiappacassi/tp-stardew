#include "game_loader.h"

#include <stdexcept>

#include "common/map/map_loader.h"
#include "world/world_manager.h"

std::filesystem::path GameLoader::resolveMapPath(const std::string& mapPath) {
    const std::filesystem::path relative(mapPath);
    if (relative.empty() || relative.is_absolute() || relative.extension() != ".toml")
        throw std::invalid_argument("Invalid map path: " + mapPath);

    auto part = relative.begin();
    if (part == relative.end() || *part++ != "maps" || part == relative.end() || (*part != "predefined" && *part != "custom"))
        throw std::invalid_argument("Map must be in maps/predefined or maps/custom");
    for (const auto& component: relative) {
        if (component == "." || component == "..")
            throw std::invalid_argument("Map path cannot contain traversal components");
    }

    auto directory = std::filesystem::current_path();
    while (true) {
        const auto candidate = directory / relative;
        if (std::filesystem::is_regular_file(candidate)) {
            const auto canonicalRoot = std::filesystem::weakly_canonical(directory / "maps");
            const auto canonicalFile = std::filesystem::canonical(candidate);
            const auto inside = canonicalFile.lexically_relative(canonicalRoot);
            if (inside.empty() || inside.is_absolute() || *inside.begin() == "..")
                throw std::invalid_argument("Map file is outside the maps directory");
            return canonicalFile;
        }
        if (directory == directory.root_path())
            break;
        directory = directory.parent_path();
    }
    throw std::runtime_error("Map TOML not found: " + mapPath);
}

PreparedWorld GameLoader::prepare(const std::string& mapPath, uint8_t worldId) {
    const auto data = MapLoader{}.load(resolveMapPath(mapPath));
    return WorldManager::prepareMap(data, worldId);
}

void GameLoader::initialize(WorldManager& world, const WorldSetup& setup) {
    if (!world.loadDungeon(setup.worldId, setup.layout))
        throw std::invalid_argument("Could not load the game dungeon");
    if (!world.loadFarm(setup.worldId, setup.farm))
        throw std::invalid_argument("Could not load the game farm");
    if (setup.townConfig.map.width != 0 && !world.loadTown(setup.townConfig))
        throw std::invalid_argument("The town or its connection to the farm is invalid");
    if (setup.entrance && !world.configureDungeonEntrance(*setup.entrance))
        throw std::invalid_argument("The dungeon entrance is invalid");
}
