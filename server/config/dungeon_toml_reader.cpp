#include "config/dungeon_toml_reader.h"

#include <cstddef>
#include <set>
#include <string>
#include <utility>

#include "config/toml_helpers.h"

ResourceType DungeonTomlReader::resourceTypeFromName(
    const std::string& name, const std::string& field) {
    if (name == "CoalRock") return ResourceType::CoalRock;
    if (name == "LapisRock") return ResourceType::LapisRock;
    if (name == "SilverRock") return ResourceType::SilverRock;
    if (name == "GoldRock") return ResourceType::GoldRock;

    toml_helpers::invalid(field, "unknown mineral type");
}

ItemId DungeonTomlReader::rewardItemFromName(
    const std::string& name, const std::string& field) {
    if (name == "Coal") return ItemId::Coal;
    if (name == "Lapis") return ItemId::Lapis;
    if (name == "Silver") return ItemId::Silver;
    if (name == "Gold") return ItemId::Gold;

    toml_helpers::invalid(field, "unknown reward item");
}

Quality DungeonTomlReader::qualityFromName(
    const std::string& name, const std::string& field) {
    if (name == "None") return Quality::None;

    toml_helpers::invalid(field, "unknown quality");
}

DungeonConfig DungeonTomlReader::read(const toml::table& root) const {
    using toml_helpers::integer;
    using toml_helpers::invalid;
    using toml_helpers::string;
    using toml_helpers::tableAt;

    const auto& dungeon = tableAt(root.get("dungeon"), "dungeon");

    DungeonConfig config;
    config.miningRange = static_cast<uint16_t>(integer(
        dungeon, "mining_range", 65535, "dungeon.mining_range"));

    if (config.miningRange == 0) {
        invalid("dungeon.mining_range", "must be greater than zero");
    }

    config.mineralDensityPercent = static_cast<uint8_t>(integer(
        dungeon, "mineral_density_percent", 100,
        "dungeon.mineral_density_percent"));

    config.minerals.clear();
    config.mineralDensityByRoom.clear();

    const auto* densityNode = dungeon.get("mineral_density_by_room");
    if (!densityNode || !densityNode->is_array()) {
        invalid("dungeon.mineral_density_by_room", "expected an array");
    }

    const auto& densities = *densityNode->as_array();
    for (std::size_t i = 0; i < densities.size(); ++i) {
        const auto field = "dungeon.mineral_density_by_room[" +
                           std::to_string(i) + "]";
        const auto& density = tableAt(&densities[i], field);

        const auto room = static_cast<uint8_t>(integer(
            density, "room", DUNGEON_ROOM_COUNT, field + ".room"));
        const auto percentage = static_cast<uint8_t>(integer(
            density, "density", 100, field + ".density"));

        if (room == 0) {
            invalid(field + ".room", "room number must be greater than zero");
        }

        if (!config.mineralDensityByRoom.emplace(room, percentage).second) {
            invalid(field, "duplicate room");
        }
    }

    const auto* mineralsNode = dungeon.get("minerals");
    if (!mineralsNode || !mineralsNode->is_array()) {
        invalid("dungeon.minerals", "expected an array");
    }

    const auto& minerals = *mineralsNode->as_array();
    if (minerals.empty()) {
        invalid("dungeon.minerals", "at least one mineral is required");
    }

    config.minerals.reserve(minerals.size());
    uint64_t totalSpawnWeight = 0;

    for (std::size_t i = 0; i < minerals.size(); ++i) {
        const auto field = "dungeon.minerals[" + std::to_string(i) + "]";
        const auto& mineral = tableAt(&minerals[i], field);

        const auto typeName = string(mineral, "type", field + ".type");
        const auto type = resourceTypeFromName(typeName, field + ".type");

        const auto hits = integer(
            mineral, "hits_to_break", 65535, field + ".hits_to_break");
        if (hits == 0) {
            invalid(field + ".hits_to_break", "must be greater than zero");
        }

        const auto weight = integer(
            mineral, "spawn_weight", 65535, field + ".spawn_weight");
        totalSpawnWeight += static_cast<uint64_t>(weight);

        const auto firstRoom = integer(
            mineral, "first_room", DUNGEON_ROOM_COUNT, field + ".first_room");
        if (firstRoom == 0) {
            invalid(field + ".first_room",
                    "room number must be greater than zero");
        }

        const auto itemName = string(
            mineral, "reward_item", field + ".reward_item");
        const auto item = rewardItemFromName(itemName, field + ".reward_item");

        const auto quantity = integer(
            mineral, "reward_quantity", 65535, field + ".reward_quantity");
        if (quantity == 0) {
            invalid(field + ".reward_quantity", "must be greater than zero");
        }

        const auto qualityName = string(
            mineral, "reward_quality", field + ".reward_quality");
        const auto quality = qualityFromName(
            qualityName, field + ".reward_quality");

        const auto chance = integer(
            mineral, "reward_chance", 100, field + ".reward_chance");

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
        invalid("dungeon.minerals", "at least one mineral must have a positive spawn_weight");
    }

    return config;
}
