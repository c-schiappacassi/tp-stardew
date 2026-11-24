#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

#include "world/contracts/world_setup.h"

class WorldManager;

class GameLoader {
private:
    static std::filesystem::path resolveMapPath(const std::string& mapPath);

public:
    static PreparedWorld prepare(const std::string& mapPath, uint8_t worldId);
    static void initialize(WorldManager& world, const WorldSetup& setup);
};
