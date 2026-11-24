#include "mineral_generator.h"

MineralGenerator::MineralGenerator(const DungeonConfig& config, uint32_t seed): definitions(config.minerals), engine(seed) {}

const MineralDefinition* MineralGenerator::choose(uint8_t roomNumber) {
    std::vector<double> weights;
    weights.reserve(definitions.size());  // es para que el push back no este re dimensionando
    double totalWeight = 0.0;

    for (const auto& definition: definitions) {
        const double weight = roomNumber >= definition.firstRoom ? definition.spawnWeight : 0.0;
        weights.push_back(weight);
        totalWeight += weight;
    }

    if (totalWeight == 0.0) {
        return nullptr;
    }

    std::discrete_distribution<std::size_t> distribution(weights.begin(), weights.end());
    return &definitions[distribution(engine)];
}

std::optional<Mineral> MineralGenerator::create(EntityId id, uint8_t roomNumber, const Position& position) {
    if (id == 0) {
        return std::nullopt;
    }

    const auto* definition = choose(roomNumber);
    if (definition == nullptr) {
        return std::nullopt;
    }

    return Mineral(id, position, definition->type, definition->hitsToBreak);
}

std::optional<Stack> MineralGenerator::rollReward(ResourceType type) {
    for (const auto& definition: definitions) {
        if (definition.type != type) {
            continue;
        }

        std::uniform_int_distribution<int> distribution(1, 100);
        if (distribution(engine) > definition.rewardChance) {
            return std::nullopt;
        }

        return definition.reward;
    }

    return std::nullopt;
}