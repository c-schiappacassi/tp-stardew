#pragma once

#include <cstdint>
#include <optional>
#include <random>
#include <vector>

#include "dungeon/dungeon_config.h"

#include "mineral.h"

class MineralGenerator {
private:
    std::vector<MineralDefinition> definitions;
    std::mt19937 engine;

    const MineralDefinition* choose(uint8_t roomNumber);

public:
    explicit MineralGenerator(const DungeonConfig& config = {}, uint32_t seed = std::random_device{}());

    std::optional<Mineral> create(EntityId id, uint8_t roomNumber, const Position& position);
    std::optional<Stack> rollReward(ResourceType type);
};