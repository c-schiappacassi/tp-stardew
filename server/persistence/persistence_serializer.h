#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include <toml++/toml.h>

#include "persistence_dto.h"
#include "persistence_index.h"

class PersistenceSerializer {
public:
    static toml::table serialize(const PersistenceDTO& dto);
    static toml::table serialize(const GameIndexDTO& dto);

private:
    static int64_t checkedInteger(uint64_t value);
    static toml::table uint64Map(const std::map<uint32_t, uint64_t>& values);
    static toml::table uint32Map(const std::map<uint32_t, uint32_t>& values);
    static toml::table boolMap(const std::map<uint32_t, bool>& values);

    static toml::table position(const Position& pos);
    static toml::table color(const Color& value);
    static toml::table stack(const Stack& value);
    static toml::table furniture(const FurnitureData& value);
    static toml::table loot(const LootData& value);
    static toml::table crop(const CropData& value);
    static toml::table animal(const AnimalData& value);
    static toml::array strings(const std::vector<std::string>& values);
    static toml::table farm(const FarmData& value);
    static toml::table enemy(const EnemySnapshot& value);
    static toml::table projectile(const ProjectileData& value);
    static toml::table trap(const TrapData& value);
    static toml::table combat(const DungeonCombatData& value);
    static toml::table mineral(const MineralData& value);
    static toml::table level(const LevelData& value);
    static toml::table mine(const MineData& value);
    static toml::table interior(const InteriorData& value);
    static toml::table zone(const ZoneData& value);
    static toml::table combatProgress(const CombatProgressData& value);
    static toml::table player(const PlayerDataLocal& value);
    static toml::table communityProgress(
        const std::map<std::string,
                       std::map<std::string,
                                std::map<std::string, uint32_t>>>& values);
};
