#include "combat_toml_reader.h"

#include "server_constants.h"

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
        throw std::invalid_argument(server_constants::INVALID_COMBAT_VALUE_PREFIX + std::string(name));
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
    for (const auto* name: {server_constants::ENEMIES, server_constants::COMBAT, server_constants::SPAWNS, server_constants::PLAYER}) {
        if (table.contains(name) && !table[name].is_table()) {
            throw std::invalid_argument(server_constants::EXPECTED_A_COMBAT_TABLE_PREFIX + std::string(name));
        }
    }
    auto nextDungeon = dungeon;
    auto nextPlayer = player;
    auto& enemies = nextDungeon.enemies;
    const std::pair<const char*, EnemyType> types[]{{server_constants::SKELETON, EnemyType::Skeleton},{server_constants::ORC, EnemyType::Orc},{server_constants::VAMPIRE_MALE, EnemyType::VampireMale},{server_constants::VAMPIRE_FEMALE, EnemyType::VampireFemale},{server_constants::SKULL, EnemyType::Skull}};
    for (const auto& [name, type]: types) {
        auto section = table[server_constants::ENEMIES][name];
        if (section && !section.is_table()) {
            throw std::invalid_argument(server_constants::EXPECTED_AN_ENEMY_TABLE_PREFIX + std::string(name));
        }
        auto& definition = enemies.definitions.at(type);
        number(section, server_constants::HEALTH_2, definition.health);
        number(section, server_constants::DAMAGE, definition.damage);
        number(section, server_constants::SPEED, definition.speed);
        number(section, server_constants::RANGE, definition.range);
        number(section, server_constants::ATTACK_MS, definition.attackMs);
    }
    auto section = table[server_constants::COMBAT];
    number(section, server_constants::SKELETON_REVIVE_MS, enemies.skeletonReviveMs);
    number(section, server_constants::VAMPIRE_REVIVE_MS, enemies.vampireReviveMs);
    number(section, server_constants::BLOOD_RADIUS, enemies.bloodRadius);
    number(section, server_constants::FLAME_SPEED, enemies.flameSpeed);
    number(section, server_constants::FLAME_LIFETIME_MS, enemies.flameLifetimeMs);
    number(section, server_constants::TRAP_INACTIVE_MS, enemies.trapInactiveMs);
    number(section, server_constants::TRAP_WARNING_MS, enemies.trapWarningMs);
    number(section, server_constants::TRAP_ACTIVE_MS, enemies.trapActiveMs);
    number(section, server_constants::TRAP_HIT_MS, enemies.trapHitMs);
    number(section, server_constants::TRAP_DAMAGE, enemies.trapDamage);
    number(section, server_constants::WALL_FLAME_RANGE, enemies.wallFlameRange);
    number(section, server_constants::ORC_SILVER_CHANCE, enemies.orcSilverChance);
    number(section, server_constants::MINING_AMBUSH_CHANCE, enemies.miningAmbushChance);
    number(section, server_constants::GOLD_CHANCE, enemies.goldChance);
    number(section, server_constants::GOLD_MIN, enemies.goldMin);
    number(section, server_constants::GOLD_MAX, enemies.goldMax);
    section = table[server_constants::SPAWNS];
    number(section, server_constants::SAFETY_RANGE, enemies.spawnSafetyRange);
    number(section, server_constants::SKELETONS_PER_ROOM, enemies.skeletonsPerRoom);
    number(section, server_constants::SPIKES_PER_ROOM, enemies.spikesPerRoom);
    number(section, server_constants::WALL_FLAMES_PER_ROOM, enemies.wallFlamesPerRoom);
    number(section, server_constants::ORC_GROUP_EVERY, enemies.orcGroupEvery);
    number(section, server_constants::VAMPIRE_PAIR_EVERY, enemies.vampirePairEvery);
    number(section, server_constants::SKULL_EVERY, enemies.skullEvery);
    number(section, server_constants::MAX_ENEMIES_PER_ROOM, enemies.maxEnemiesPerRoom);
    section = table[server_constants::PLAYER];
    number(section, server_constants::SWORD_DAMAGE, nextPlayer.swordDamage);
    number(section, server_constants::SWORD_RANGE, nextPlayer.swordRange);
    number(section, server_constants::DAMAGE_GRACE_MS, nextPlayer.damageGraceMs);
    number(section, server_constants::PASSOUT_GOLD_PERCENT, nextPlayer.passoutGoldPercent);
    number(section, server_constants::DAY_DURATION_MS, nextPlayer.dayDurationMs);
    number(section, server_constants::SWORD_ENERGY_COST, nextPlayer.tools.at(ItemId::Sword).energyCost);
    number(section, server_constants::SWORD_INTERVAL_MS, nextPlayer.tools.at(ItemId::Sword).intervalMs);
    enemies.validate();
    nextPlayer.validate();
    dungeon = std::move(nextDungeon);
    player = std::move(nextPlayer);
}
