#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include <toml++/toml.h>

#include "persistence_dto.h"
#include "persistence_index.h"

class PersistenceDeserializer {
public:
    static PersistenceDTO deserializeGame(const toml::table& table);
    static GameIndexDTO deserializeIndex(const toml::table& table);

private:
    static const toml::table& requiredTable(const toml::table& table, const char* key);
    static const toml::array& requiredArray(const toml::table& table, const char* key);
    static int64_t integer(const toml::table& table, const std::string& key,
                           int64_t min, int64_t max);
    static bool boolean(const toml::table& table, const std::string& key);
    static std::string string(const toml::table& table, const std::string& key);
    static uint32_t numericKey(const std::string& key);
    static std::map<uint32_t, uint64_t> uint64Map(const toml::table& table);
    static std::map<uint32_t, uint32_t> uint32Map(const toml::table& table);
    static std::map<uint32_t, bool> boolMap(const toml::table& table);

    static GameIndexPlayerDTO indexPlayer(const toml::table& table);
    static Position position(const toml::table& table);
    static Color color(const toml::table& table);
    static Stack stack(const toml::table& table);
    static FurnitureData furniture(const toml::table& table, bool decorationsInTiles = false);
    static LootData loot(const toml::table& table);
    static CropData crop(const toml::table& table);
    static AnimalData animal(const toml::table& table);
    static std::vector<std::string> strings(const toml::array& array);
    static FarmData farm(const toml::table& table);
    static EnemySnapshot enemy(const toml::table& table);
    static ProjectileData projectile(const toml::table& table);
    static TrapData trap(const toml::table& table);
    static DungeonCombatData combat(const toml::table& table);
    static MineralData mineral(const toml::table& table);
    static LevelData level(const toml::table& table);
    static MineData mine(const toml::table& table);
    static InteriorData interior(const toml::table& table);
    static ZoneData zone(const toml::table& table);
    static CombatProgressData combatProgress(const toml::table& table);
    static PlayerDataLocal player(const toml::table& table);
    static CommunitySaveData communityProgress(const toml::table& table);
};
