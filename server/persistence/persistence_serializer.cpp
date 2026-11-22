#include "persistence_serializer.h"

#include <limits>
#include <stdexcept>
#include <utility>
#include <variant>

int64_t PersistenceSerializer::checkedInteger(uint64_t value) {
    if (value > static_cast<uint64_t>(std::numeric_limits<int64_t>::max())) {
        throw std::overflow_error("TOML cannot represent an unsigned integer above INT64_MAX");
    }
    return static_cast<int64_t>(value);
}

toml::table PersistenceSerializer::uint64Map(const std::map<uint32_t, uint64_t>& values) {
    toml::table result;
    for (const auto& [id, value] : values) {
        result.insert(std::to_string(id), checkedInteger(value));
    }
    return result;
}

toml::table PersistenceSerializer::uint32Map(const std::map<uint32_t, uint32_t>& values) {
    toml::table result;
    for (const auto& [id, value] : values) {
        result.insert(std::to_string(id), static_cast<int64_t>(value));
    }
    return result;
}

toml::table PersistenceSerializer::boolMap(const std::map<uint32_t, bool>& values) {
    toml::table result;
    for (const auto& [id, value] : values) {
        result.insert(std::to_string(id), value);
    }
    return result;
}

toml::table PersistenceSerializer::position(const Position& pos) {
    return toml::table{{"x", pos.getX()}, {"y", pos.getY()}};
}

toml::table PersistenceSerializer::color(const Color& value) {
    return toml::table{{"r", static_cast<int64_t>(value.r)}, {"g", static_cast<int64_t>(value.g)}, {"b", static_cast<int64_t>(value.b)}};
}

toml::table PersistenceSerializer::stack(const Stack& value) {
    return toml::table{{"itemId", static_cast<int64_t>(value.item_id)},
            {"quantity", static_cast<int64_t>(value.quantity)},
            {"quality", static_cast<int64_t>(value.qual)}};
}

toml::table PersistenceSerializer::furniture(const FurnitureData& value) {
    return toml::table{{"id", static_cast<int64_t>(value.id)}, {"pos", position(value.pos)},
            {"type", static_cast<int64_t>(value.type)}, {"dir", static_cast<int64_t>(value.dir)}};
}

toml::table PersistenceSerializer::loot(const LootData& value) {
    return toml::table{{"id", static_cast<int64_t>(value.id)}, {"pos", position(value.pos)},
            {"item", static_cast<int64_t>(value.item)}, {"quantity", static_cast<int64_t>(value.quantity)},
            {"quality", static_cast<int64_t>(value.quality)}, {"kind", static_cast<int64_t>(value.kind)}};
}

toml::table PersistenceSerializer::crop(const CropData& value) {
    return toml::table{{"id", static_cast<int64_t>(value.id)}, {"pos", position(value.pos)},
            {"type", static_cast<int64_t>(value.type)}, {"state", static_cast<int64_t>(value.state)},
            {"wattered", value.wattered}};
}

toml::table PersistenceSerializer::animal(const AnimalData& value) {
    return toml::table{{"id", static_cast<int64_t>(value.id)}, {"pos", position(value.pos)},
            {"dir", static_cast<int64_t>(value.dir)}, {"type", static_cast<int64_t>(value.type)},
            {"state", static_cast<int64_t>(value.state)},
            {"needsInteraction", value.needs_interaction}};
}

toml::array PersistenceSerializer::strings(const std::vector<std::string>& values) {
    toml::array result;
    for (const std::string& value : values) {
        result.push_back(value);
    }
    return result;
}

toml::table PersistenceSerializer::farm(const FarmData& value) {
    toml::table result;
    toml::array crops;
    for (const auto& item : value.crops) {
        crops.push_back(crop(item));
    }
    result.insert("crops", std::move(crops));
    toml::array animals;
    for (const auto& item : value.animals) {
        animals.push_back(animal(item));
    }
    result.insert("animals", std::move(animals));
    toml::array tilledSoil;
    for (const auto& item : value.tilledSoil) {
        tilledSoil.push_back(position(item));
    }
    result.insert("tilledSoil", std::move(tilledSoil));
    toml::array furnitureArray;
    for (const auto& item : value.furniture) {
        furnitureArray.push_back(furniture(item));
    }
    result.insert("furniture", std::move(furnitureArray));
    toml::array lootArray;
    for (const auto& item : value.loot) {
        lootArray.push_back(loot(item));
    }
    result.insert("loot", std::move(lootArray));
    return result;
}

toml::table PersistenceSerializer::enemy(const EnemySnapshot& value) {
    return toml::table{{"id", static_cast<int64_t>(value.id)}, {"pos", position(value.pos)},
            {"dir", static_cast<int64_t>(value.dir)}, {"type", static_cast<int64_t>(value.type)},
            {"state", static_cast<int64_t>(value.state)}, {"health", static_cast<int64_t>(value.health)},
            {"maxHealth", static_cast<int64_t>(value.maxHealth)}, {"frenzied", value.frenzied},
            {"action", static_cast<int64_t>(value.action)}};
}

toml::table PersistenceSerializer::projectile(const ProjectileData& value) {
    return toml::table{{"id", static_cast<int64_t>(value.id)}, {"pos", position(value.pos)},
            {"type", static_cast<int64_t>(value.type)}};
}

toml::table PersistenceSerializer::trap(const TrapData& value) {
    return toml::table{{"id", static_cast<int64_t>(value.id)}, {"pos", position(value.pos)},
            {"dir", static_cast<int64_t>(value.dir)}, {"type", static_cast<int64_t>(value.type)},
            {"state", static_cast<int64_t>(value.state)}, {"range", static_cast<int64_t>(value.range)}};
}

toml::table PersistenceSerializer::combat(const DungeonCombatData& value) {
    toml::table result;
    result.insert("visitId", static_cast<int64_t>(value.visitId));
    result.insert("room", static_cast<int64_t>(value.room));
    toml::array enemies;
    for (const auto& item : value.enemies) {
        enemies.push_back(enemy(item));
    }
    result.insert("enemies", std::move(enemies));
    toml::array projectiles;
    for (const auto& item : value.projectiles) {
        projectiles.push_back(projectile(item));
    }
    result.insert("projectiles", std::move(projectiles));
    toml::array traps;
    for (const auto& item : value.traps) {
        traps.push_back(trap(item));
    }
    result.insert("traps", std::move(traps));
    toml::array lootArray;
    for (const auto& item : value.loot) {
        lootArray.push_back(loot(item));
    }
    result.insert("loot", std::move(lootArray));
    return result;
}

toml::table PersistenceSerializer::mineral(const MineralData& value) {
    return toml::table{{"id", static_cast<int64_t>(value.id)}, {"pos", position(value.pos)},
            {"type", static_cast<int64_t>(value.type)}, {"state", static_cast<int64_t>(value.state)}};
}

toml::table PersistenceSerializer::level(const LevelData& value) {
    toml::table result;
    result.insert("entry", position(value.entry));
    result.insert("exit", position(value.exit));
    toml::array minerals;
    for (const auto& item : value.minerals) {
        minerals.push_back(mineral(item));
    }
    result.insert("minerals", std::move(minerals));
    result.insert("combat", combat(value.combat));
    return result;
}

toml::table PersistenceSerializer::mine(const MineData& value) {
    return toml::table{{"currentLevel", static_cast<int64_t>(value.current_level)},
            {"currentLevelData", level(value.current_level_data)}};
}

toml::table PersistenceSerializer::interior(const InteriorData& value) {
    toml::table result;
    result.insert("interiorId", static_cast<int64_t>(value.interior_id));
    result.insert("houseIndex", static_cast<int64_t>(value.house_index));
    toml::array furnitureArray;
    for (const auto& item : value.furniture) {
        furnitureArray.push_back(furniture(item));
    }
    result.insert("furniture", std::move(furnitureArray));
    return result;
}

toml::table PersistenceSerializer::zone(const ZoneData& value) {
    if (const FarmData* data = std::get_if<FarmData>(&value)) {
        return toml::table{{"type", "farm"}, {"data", farm(*data)}};
    }
    if (const MineData* data = std::get_if<MineData>(&value)) {
        return toml::table{{"type", "mine"}, {"data", mine(*data)}};
    }
    if (const InteriorData* data = std::get_if<InteriorData>(&value)) {
        return toml::table{{"type", "interior"}, {"data", interior(*data)}};
    }
    return toml::table{{"type", "none"}};
}

toml::table PersistenceSerializer::combatProgress(const CombatProgressData& value) {
    return toml::table{{"skeletons", value.skeletons}, {"vampires", value.vampires},
            {"orcs", value.orcs}, {"skull", value.skull}};
}

toml::table PersistenceSerializer::player(const PlayerDataLocal& value) {
    toml::table result;
    result.insert("characterName", value.characterName);
    result.insert("pos", position(value.pos));
    result.insert("state", static_cast<int64_t>(value.state));
    result.insert("dir", static_cast<int64_t>(value.dir));
    result.insert("hairModel", static_cast<int64_t>(value.hairModel));
    result.insert("shirtModel", static_cast<int64_t>(value.shirtModel));
    result.insert("pantsModel", static_cast<int64_t>(value.pantsModel));
    result.insert("hairColor", color(value.hairColor));
    result.insert("pantsColor", color(value.pantsColor));
    result.insert("currentZone", static_cast<int64_t>(value.currentZone));
    result.insert("dungeonLevel", static_cast<int64_t>(value.dungeonLevel));
    if (value.interior) {
        result.insert("interior", toml::table{
            {"interiorId", static_cast<int64_t>(value.interior->interior_id)},
            {"houseIndex", static_cast<int64_t>(value.interior->house_index)}
        });
    }
    if (value.heldItem) {
        result.insert("heldItem", static_cast<int64_t>(*value.heldItem));
    }
    toml::table items;
    for (const auto& [slot, item] : value.items) {
        items.insert(std::to_string(slot), stack(item));
    }
    result.insert("items", std::move(items));
    result.insert("currentSlot", static_cast<int64_t>(value.current_slot));
    result.insert("inventoryCapacity", static_cast<int64_t>(value.cap_inventory));
    result.insert("gold", static_cast<int64_t>(value.gold));
    result.insert("health", static_cast<int64_t>(value.health));
    result.insert("maxHealth", static_cast<int64_t>(value.maxHealth));
    result.insert("energy", static_cast<int64_t>(value.energy));
    result.insert("maxEnergy", static_cast<int64_t>(value.maxEnergy));
    result.insert("zoneData", zone(value.zoneData));
    result.insert("combatProgress", combatProgress(value.combatProgress));
    toml::array messages;
    for (const ChatMessageData& msg : value.newMessages) {
        messages.push_back(toml::table{{"sender", msg.sender}, {"text", msg.text}});
    }
    result.insert("newMessages", std::move(messages));
    return result;
}

toml::table PersistenceSerializer::communityProgress(const CommunitySaveData& values) {
    toml::table result;
    for (const auto& [task, actions] : values) {
        toml::table actionTable;
        for (const auto& [action, counters] : actions) {
            toml::table counterTable;
            for (const auto& [name, count] : counters) {
                counterTable.insert(name, static_cast<int64_t>(count));
            }
            actionTable.insert(action, std::move(counterTable));
        }
        result.insert(task, std::move(actionTable));
    }
    return result;
}


toml::table PersistenceSerializer::indexPlayer(const GameIndexPlayerDTO& value) {
    toml::table result;
    result.insert("characterName", value.characterName);
    result.insert("hairModel", static_cast<int64_t>(value.hairModel));
    result.insert("shirtModel", static_cast<int64_t>(value.shirtModel));
    result.insert("pantsModel", static_cast<int64_t>(value.pantsModel));
    result.insert("hairColor", color(value.hairColor));
    result.insert("pantsColor", color(value.pantsColor));
    return result;
}

toml::table PersistenceSerializer::serialize(const GameIndexDTO& dto) {
    toml::table result;
    result.insert("id", dto.id);
    result.insert("farmName", dto.farmName);
    result.insert("mapPath", dto.mapPath);
    result.insert("users", strings(dto.users));
    result.insert("isMultiplayer", dto.isMultiplayer);

    toml::table previews;
    for (const auto& [userName, player] : dto.playerPreviews) {
        previews.insert(userName, indexPlayer(player));
    }
    result.insert("playerPreviews", std::move(previews));
    return result;
}

toml::table PersistenceSerializer::serialize(const PersistenceDTO& dto) {
    toml::table result;
    result.insert("id", dto.id);
    result.insert("farmName", dto.farmName);
    result.insert("mapId", static_cast<int64_t>(dto.mapId));
    result.insert("currentDay", static_cast<int64_t>(dto.currentDay));
    result.insert("users", strings(dto.users));

    toml::table players;
    for (const auto& [username, data] : dto.players) {
        players.insert(username, player(data));
    }
    result.insert("players", std::move(players));
    result.insert("farm", farm(dto.farm));
    toml::array emptySoilElapsed;
    for (const auto& [pos, elapsed] : dto.emptySoilElapsed) {
        emptySoilElapsed.push_back(toml::table{{"pos", position(pos)},
            {"elapsed", checkedInteger(elapsed)}});
    }
    result.insert("emptySoilElapsed", std::move(emptySoilElapsed));
    result.insert("cropGrowthProgress", uint64Map(dto.cropGrowthProgress));
    result.insert("cropWaterRemainingTime", uint64Map(dto.cropWaterRemainingTime));
    result.insert("harvestedCrops", boolMap(dto.harvestedCrops));
    result.insert("regrowingCrops", boolMap(dto.regrowingCrops));
    result.insert("animalProductionProgress", uint64Map(dto.animalProductionProgress));
    result.insert("animalAvailableProducts", uint32Map(dto.animalAvailableProducts));
    result.insert("communityCenterActionProgress", communityProgress(dto.communityCenterActionProgress));
    return result;
}
