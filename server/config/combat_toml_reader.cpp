#include "combat_toml_reader.h"

#include <limits>
#include <stdexcept>
#include <string_view>
#include <utility>

namespace {
uint32_t readNumber(const toml::node_view<const toml::node>& section, std::string_view name,
                    uint32_t current, uint32_t maximum) {
    const auto node = section[name];
    if (!node) {
        return current;
    }
    const auto value = node.value<int64_t>();
    if (!node.is_integer() || !value || *value < 0 || static_cast<uint64_t>(*value) > maximum) {
        throw std::invalid_argument("Invalid combat value: " + std::string(name));
    }
    return static_cast<uint32_t>(*value);
}

void number(const toml::node_view<const toml::node>& section, std::string_view name,
            uint8_t& target) {
    target = static_cast<uint8_t>(
            readNumber(section, name, target, std::numeric_limits<uint8_t>::max()));
}

void number(const toml::node_view<const toml::node>& section, std::string_view name,
            uint16_t& target) {
    target = static_cast<uint16_t>(
            readNumber(section, name, target, std::numeric_limits<uint16_t>::max()));
}

void number(const toml::node_view<const toml::node>& section, std::string_view name,
            uint32_t& target) {
    target = readNumber(section, name, target, std::numeric_limits<uint32_t>::max());
}
}

void CombatTomlReader::read(const toml::table& table, DungeonConfig& dungeon,
                            PlayerConfig& player) const {
    for (const auto* name: {"enemies", "combat", "spawns", "player"}) {
        if (table.contains(name) && !table[name].is_table()) {
            throw std::invalid_argument("Expected a combat table: " + std::string(name));
        }
    }
    auto nextDungeon = dungeon;
    auto nextPlayer = player;
    auto& enemies = nextDungeon.enemies;
    const std::pair<const char*, EnemyType> types[]{{"skeleton", EnemyType::Skeleton},{"orc", EnemyType::Orc},{"vampire_male", EnemyType::VampireMale},{"vampire_female", EnemyType::VampireFemale},{"skull", EnemyType::Skull}};
    for (const auto& [name, type]: types) {
        auto section = table["enemies"][name];
        if (section && !section.is_table()) {
            throw std::invalid_argument("Expected an enemy table: " + std::string(name));
        }
        auto& definition = enemies.definitions.at(type);
        number(section, "health", definition.health);
        number(section, "damage", definition.damage);
        number(section, "speed", definition.speed);
        number(section, "range", definition.range);
        number(section, "attack_ms", definition.attackMs);
    }
    auto section = table["combat"];
    number(section, "skeleton_revive_ms", enemies.skeletonReviveMs);
    number(section, "vampire_revive_ms", enemies.vampireReviveMs);
    number(section, "blood_radius", enemies.bloodRadius);
    number(section, "flame_speed", enemies.flameSpeed);
    number(section, "flame_lifetime_ms", enemies.flameLifetimeMs);
    number(section, "trap_inactive_ms", enemies.trapInactiveMs);
    number(section, "trap_warning_ms", enemies.trapWarningMs);
    number(section, "trap_active_ms", enemies.trapActiveMs);
    number(section, "trap_hit_ms", enemies.trapHitMs);
    number(section, "trap_damage", enemies.trapDamage);
    number(section, "wall_flame_range", enemies.wallFlameRange);
    number(section, "orc_silver_chance", enemies.orcSilverChance);
    number(section, "mining_ambush_chance", enemies.miningAmbushChance);
    number(section, "gold_chance", enemies.goldChance);
    number(section, "gold_min", enemies.goldMin);
    number(section, "gold_max", enemies.goldMax);
    section = table["spawns"];
    number(section, "safety_range", enemies.spawnSafetyRange);
    number(section, "skeletons_per_room", enemies.skeletonsPerRoom);
    number(section, "spikes_per_room", enemies.spikesPerRoom);
    number(section, "wall_flames_per_room", enemies.wallFlamesPerRoom);
    number(section, "orc_group_every", enemies.orcGroupEvery);
    number(section, "vampire_pair_every", enemies.vampirePairEvery);
    number(section, "skull_every", enemies.skullEvery);
    number(section, "max_enemies_per_room", enemies.maxEnemiesPerRoom);
    section = table["player"];
    number(section, "sword_damage", nextPlayer.swordDamage);
    number(section, "sword_range", nextPlayer.swordRange);
    number(section, "damage_grace_ms", nextPlayer.damageGraceMs);
    number(section, "passout_gold_percent", nextPlayer.passoutGoldPercent);
    number(section, "day_duration_ms", nextPlayer.dayDurationMs);
    number(section, "sword_energy_cost", nextPlayer.tools.at(ItemId::Sword).energyCost);
    number(section, "sword_interval_ms", nextPlayer.tools.at(ItemId::Sword).intervalMs);
    enemies.validate();
    nextPlayer.validate();
    dungeon = std::move(nextDungeon);
    player = std::move(nextPlayer);
}
