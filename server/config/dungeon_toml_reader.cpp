#include "config/dungeon_toml_reader.h"

#include "server_constants.h"

#include <cstddef>
#include <set>
#include <string>
#include <utility>

#include "config/toml_helpers.h"

ResourceType DungeonTomlReader::resourceTypeFromName(
    const std::string& name, const std::string& field) {
    if (name == server_constants::COALROCK) return ResourceType::CoalRock;
    if (name == server_constants::LAPISROCK) return ResourceType::LapisRock;
    if (name == server_constants::SILVERROCK) return ResourceType::SilverRock;
    if (name == server_constants::GOLDROCK) return ResourceType::GoldRock;

    toml_helpers::invalid(field, server_constants::UNKNOWN_MINERAL_TYPE);
}

ItemId DungeonTomlReader::rewardItemFromName(
    const std::string& name, const std::string& field) {
    if (name == server_constants::COAL) return ItemId::Coal;
    if (name == server_constants::LAPIS) return ItemId::Lapis;
    if (name == server_constants::SILVER) return ItemId::Silver;
    if (name == server_constants::GOLD) return ItemId::Gold;

    toml_helpers::invalid(field, server_constants::UNKNOWN_REWARD_ITEM);
}

Quality DungeonTomlReader::qualityFromName(
    const std::string& name, const std::string& field) {
    if (name == server_constants::NONE) return Quality::None;

    toml_helpers::invalid(field, server_constants::UNKNOWN_QUALITY_2);
}

DungeonConfig DungeonTomlReader::read(const toml::table& root) const {
    using toml_helpers::integer;
    using toml_helpers::invalid;
    using toml_helpers::string;
    using toml_helpers::tableAt;

    const auto& dungeon = tableAt(root.get(server_constants::DUNGEON), server_constants::DUNGEON);

    DungeonConfig config;
    config.miningRange = static_cast<uint16_t>(integer(
        dungeon, server_constants::MINING_RANGE, 65535, server_constants::DUNGEON_MINING_RANGE));

    if (config.miningRange == 0) {
        invalid(server_constants::DUNGEON_MINING_RANGE, server_constants::MUST_BE_GREATER_THAN_ZERO);
    }

    config.mineralDensityPercent = static_cast<uint8_t>(integer(
        dungeon, server_constants::MINERAL_DENSITY_PERCENT, 100,
        server_constants::DUNGEON_MINERAL_DENSITY_PERCENT));

    config.minerals.clear();
    config.mineralDensityByRoom.clear();

    const auto* densityNode = dungeon.get(server_constants::MINERAL_DENSITY_BY_ROOM);
    if (!densityNode || !densityNode->is_array()) {
        invalid(server_constants::DUNGEON_MINERAL_DENSITY_BY_ROOM, server_constants::EXPECTED_AN_ARRAY);
    }

    const auto& densities = *densityNode->as_array();
    for (std::size_t i = 0; i < densities.size(); ++i) {
        const auto field = server_constants::DUNGEON_MINERAL_DENSITY_BY_ROOM_PREFIX +
                           std::to_string(i) + server_constants::CLOSE_BRACKET;
        const auto& density = tableAt(&densities[i], field);

        const auto room = static_cast<uint8_t>(integer(
            density, server_constants::ROOM, DUNGEON_ROOM_COUNT, field + server_constants::FIELD_ROOM));
        const auto percentage = static_cast<uint8_t>(integer(
            density, server_constants::DENSITY, 100, field + server_constants::FIELD_DENSITY));

        if (room == 0) {
            invalid(field + server_constants::FIELD_ROOM, server_constants::ROOM_NUMBER_MUST_BE_GREATER_THAN_ZERO);
        }

        if (!config.mineralDensityByRoom.emplace(room, percentage).second) {
            invalid(field, server_constants::DUPLICATE_ROOM);
        }
    }

    const auto* mineralsNode = dungeon.get(server_constants::MINERALS);
    if (!mineralsNode || !mineralsNode->is_array()) {
        invalid(server_constants::DUNGEON_MINERALS, server_constants::EXPECTED_AN_ARRAY);
    }

    const auto& minerals = *mineralsNode->as_array();
    if (minerals.empty()) {
        invalid(server_constants::DUNGEON_MINERALS, server_constants::AT_LEAST_ONE_MINERAL_IS_REQUIRED);
    }

    config.minerals.reserve(minerals.size());
    uint64_t totalSpawnWeight = 0;

    for (std::size_t i = 0; i < minerals.size(); ++i) {
        const auto field = server_constants::DUNGEON_MINERALS_PREFIX + std::to_string(i) + server_constants::CLOSE_BRACKET;
        const auto& mineral = tableAt(&minerals[i], field);

        const auto typeName = string(mineral, server_constants::TYPE, field + server_constants::FIELD_TYPE);
        const auto type = resourceTypeFromName(typeName, field + server_constants::FIELD_TYPE);

        const auto hits = integer(
            mineral, server_constants::HITS_TO_BREAK, 65535, field + server_constants::FIELD_HITS_TO_BREAK);
        if (hits == 0) {
            invalid(field + server_constants::FIELD_HITS_TO_BREAK, server_constants::MUST_BE_GREATER_THAN_ZERO);
        }

        const auto weight = integer(
            mineral, server_constants::SPAWN_WEIGHT, 65535, field + server_constants::FIELD_SPAWN_WEIGHT);
        totalSpawnWeight += static_cast<uint64_t>(weight);

        const auto firstRoom = integer(
            mineral, server_constants::FIRST_ROOM, DUNGEON_ROOM_COUNT, field + server_constants::FIELD_FIRST_ROOM);
        if (firstRoom == 0) {
            invalid(field + server_constants::FIELD_FIRST_ROOM,
                    server_constants::ROOM_NUMBER_MUST_BE_GREATER_THAN_ZERO);
        }

        const auto itemName = string(
            mineral, server_constants::REWARD_ITEM, field + server_constants::FIELD_REWARD_ITEM);
        const auto item = rewardItemFromName(itemName, field + server_constants::FIELD_REWARD_ITEM);

        const auto quantity = integer(
            mineral, server_constants::REWARD_QUANTITY, 65535, field + server_constants::FIELD_REWARD_QUANTITY);
        if (quantity == 0) {
            invalid(field + server_constants::FIELD_REWARD_QUANTITY, server_constants::MUST_BE_GREATER_THAN_ZERO);
        }

        const auto qualityName = string(
            mineral, server_constants::REWARD_QUALITY, field + server_constants::FIELD_REWARD_QUALITY);
        const auto quality = qualityFromName(
            qualityName, field + server_constants::FIELD_REWARD_QUALITY);

        const auto chance = integer(
            mineral, server_constants::REWARD_CHANCE, 100, field + server_constants::FIELD_REWARD_CHANCE);

        config.minerals.push_back({
            type,
            static_cast<uint16_t>(hits),
            static_cast<uint16_t>(weight),
            static_cast<uint8_t>(firstRoom),
            {item, static_cast<uint16_t>(quantity), quality},
            static_cast<uint8_t>(chance)
        });
    }

    if (totalSpawnWeight == 0) {
        invalid(server_constants::DUNGEON_MINERALS, server_constants::AT_LEAST_ONE_MINERAL_MUST_HAVE_A_POSITIVE_SPAWN_WEIGHT);
    }

    return config;
}
