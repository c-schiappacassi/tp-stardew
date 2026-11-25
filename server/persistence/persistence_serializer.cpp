#include "persistence_serializer.h"

#include "server_constants.h"

#include <limits>
#include <stdexcept>
#include <utility>
#include <variant>

int64_t PersistenceSerializer::checkedInteger(uint64_t value) {
    if (value > static_cast<uint64_t>(std::numeric_limits<int64_t>::max())) {
        throw std::overflow_error(server_constants::TOML_CANNOT_REPRESENT_AN_UNSIGNED_INTEGER_ABOVE_INT64_MAX);
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
    return toml::table{{server_constants::X, pos.getX()}, {server_constants::Y, pos.getY()}};
}

toml::table PersistenceSerializer::color(const Color& value) {
    return toml::table{{server_constants::R, static_cast<int64_t>(value.r)}, {server_constants::G, static_cast<int64_t>(value.g)}, {server_constants::B, static_cast<int64_t>(value.b)}};
}

toml::table PersistenceSerializer::stack(const Stack& value) {
    return toml::table{{server_constants::ITEMID, static_cast<int64_t>(value.item_id)},
            {server_constants::QUANTITY, static_cast<int64_t>(value.quantity)},
            {server_constants::QUALITY, static_cast<int64_t>(value.qual)}};
}

toml::table PersistenceSerializer::furniture(const FurnitureData& value) {
    return toml::table{{server_constants::ID, static_cast<int64_t>(value.id)}, {server_constants::POS, position(value.pos)},
            {server_constants::TYPE, static_cast<int64_t>(value.type)}, {server_constants::DIR, static_cast<int64_t>(value.dir)}};
}

toml::table PersistenceSerializer::loot(const LootData& value) {
    return toml::table{{server_constants::ID, static_cast<int64_t>(value.id)}, {server_constants::POS, position(value.pos)},
            {server_constants::ITEM, static_cast<int64_t>(value.item)}, {server_constants::QUANTITY, static_cast<int64_t>(value.quantity)},
            {server_constants::QUALITY, static_cast<int64_t>(value.quality)}, {server_constants::KIND, static_cast<int64_t>(value.kind)}};
}

toml::table PersistenceSerializer::crop(const CropData& value) {
    return toml::table{{server_constants::ID, static_cast<int64_t>(value.id)}, {server_constants::POS, position(value.pos)},
            {server_constants::TYPE, static_cast<int64_t>(value.type)}, {server_constants::STATE, static_cast<int64_t>(value.state)},
            {server_constants::WATTERED, value.wattered}};
}

toml::table PersistenceSerializer::animal(const AnimalData& value) {
    return toml::table{{server_constants::ID, static_cast<int64_t>(value.id)}, {server_constants::POS, position(value.pos)},
            {server_constants::DIR, static_cast<int64_t>(value.dir)}, {server_constants::TYPE, static_cast<int64_t>(value.type)},
            {server_constants::STATE, static_cast<int64_t>(value.state)},
            {server_constants::NEEDSINTERACTION, value.needs_interaction}};
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
    result.insert(server_constants::CROPS, std::move(crops));
    toml::array animals;
    for (const auto& item : value.animals) {
        animals.push_back(animal(item));
    }
    result.insert(server_constants::ANIMALS, std::move(animals));
    toml::array tilledSoil;
    for (const auto& item : value.tilledSoil) {
        tilledSoil.push_back(position(item));
    }
    result.insert(server_constants::TILLEDSOIL, std::move(tilledSoil));
    toml::array furnitureArray;
    for (const auto& item : value.furniture) {
        furnitureArray.push_back(furniture(item));
    }
    result.insert(server_constants::FURNITURE, std::move(furnitureArray));
    toml::array lootArray;
    for (const auto& item : value.loot) {
        lootArray.push_back(loot(item));
    }
    result.insert(server_constants::LOOT, std::move(lootArray));
    return result;
}

toml::table PersistenceSerializer::enemy(const EnemySnapshot& value) {
    return toml::table{{server_constants::ID, static_cast<int64_t>(value.id)}, {server_constants::POS, position(value.pos)},
            {server_constants::DIR, static_cast<int64_t>(value.dir)}, {server_constants::TYPE, static_cast<int64_t>(value.type)},
            {server_constants::STATE, static_cast<int64_t>(value.state)}, {server_constants::HEALTH_2, static_cast<int64_t>(value.health)},
            {server_constants::MAXHEALTH, static_cast<int64_t>(value.maxHealth)}, {server_constants::FRENZIED, value.frenzied},
            {server_constants::ACTION, static_cast<int64_t>(value.action)}};
}

toml::table PersistenceSerializer::projectile(const ProjectileData& value) {
    return toml::table{{server_constants::ID, static_cast<int64_t>(value.id)}, {server_constants::POS, position(value.pos)},
            {server_constants::TYPE, static_cast<int64_t>(value.type)}};
}

toml::table PersistenceSerializer::trap(const TrapData& value) {
    return toml::table{{server_constants::ID, static_cast<int64_t>(value.id)}, {server_constants::POS, position(value.pos)},
            {server_constants::DIR, static_cast<int64_t>(value.dir)}, {server_constants::TYPE, static_cast<int64_t>(value.type)},
            {server_constants::STATE, static_cast<int64_t>(value.state)}, {server_constants::RANGE, static_cast<int64_t>(value.range)}};
}

toml::table PersistenceSerializer::combat(const DungeonCombatData& value) {
    toml::table result;
    result.insert(server_constants::VISITID, static_cast<int64_t>(value.visitId));
    result.insert(server_constants::ROOM, static_cast<int64_t>(value.room));
    toml::array enemies;
    for (const auto& item : value.enemies) {
        enemies.push_back(enemy(item));
    }
    result.insert(server_constants::ENEMIES, std::move(enemies));
    toml::array projectiles;
    for (const auto& item : value.projectiles) {
        projectiles.push_back(projectile(item));
    }
    result.insert(server_constants::PROJECTILES, std::move(projectiles));
    toml::array traps;
    for (const auto& item : value.traps) {
        traps.push_back(trap(item));
    }
    result.insert(server_constants::TRAPS, std::move(traps));
    toml::array lootArray;
    for (const auto& item : value.loot) {
        lootArray.push_back(loot(item));
    }
    result.insert(server_constants::LOOT, std::move(lootArray));
    return result;
}

toml::table PersistenceSerializer::mineral(const MineralData& value) {
    return toml::table{{server_constants::ID, static_cast<int64_t>(value.id)}, {server_constants::POS, position(value.pos)},
            {server_constants::TYPE, static_cast<int64_t>(value.type)}, {server_constants::STATE, static_cast<int64_t>(value.state)},
            {server_constants::HEALTH_2, static_cast<int64_t>(value.health)}};
}

toml::table PersistenceSerializer::level(const LevelData& value) {
    toml::table result;
    result.insert(server_constants::ENTRY, position(value.entry));
    result.insert(server_constants::EXIT, position(value.exit));
    toml::array minerals;
    for (const auto& item : value.minerals) {
        minerals.push_back(mineral(item));
    }
    result.insert(server_constants::MINERALS, std::move(minerals));
    result.insert(server_constants::COMBAT, combat(value.combat));
    return result;
}

toml::table PersistenceSerializer::mine(const MineData& value) {
    return toml::table{{server_constants::CURRENTLEVEL, static_cast<int64_t>(value.current_level)},
            {server_constants::CURRENTLEVELDATA, level(value.current_level_data)}};
}

toml::table PersistenceSerializer::interior(const InteriorData& value) {
    toml::table result;
    result.insert(server_constants::INTERIORID, static_cast<int64_t>(value.interior_id));
    result.insert(server_constants::HOUSEINDEX, static_cast<int64_t>(value.house_index));
    toml::array furnitureArray;
    for (const auto& item : value.furniture) {
        furnitureArray.push_back(furniture(item));
    }
    result.insert(server_constants::FURNITURE, std::move(furnitureArray));
    toml::array lootArray;
    for (const auto& item: value.loot) lootArray.push_back(loot(item));
    result.insert(server_constants::LOOT, std::move(lootArray));
    return result;
}

toml::table PersistenceSerializer::zone(const ZoneData& value) {
    if (const FarmData* data = std::get_if<FarmData>(&value)) {
        return toml::table{{server_constants::TYPE, server_constants::FARM}, {server_constants::DATA, farm(*data)}};
    }
    if (const MineData* data = std::get_if<MineData>(&value)) {
        return toml::table{{server_constants::TYPE, server_constants::MINE}, {server_constants::DATA, mine(*data)}};
    }
    if (const InteriorData* data = std::get_if<InteriorData>(&value)) {
        return toml::table{{server_constants::TYPE, server_constants::INTERIOR}, {server_constants::DATA, interior(*data)}};
    }
    return toml::table{{server_constants::TYPE, server_constants::NONE_2}};
}

toml::table PersistenceSerializer::combatProgress(const CombatProgressData& value) {
    return toml::table{{server_constants::SKELETONS, value.skeletons}, {server_constants::VAMPIRES, value.vampires},
            {server_constants::ORCS, value.orcs}, {server_constants::SKULL, value.skull}};
}

toml::table PersistenceSerializer::player(const PlayerDataLocal& value) {
    toml::table result;
    result.insert(server_constants::CHARACTERNAME, value.characterName);
    result.insert(server_constants::POS, position(value.pos));
    result.insert(server_constants::STATE, static_cast<int64_t>(value.state));
    result.insert(server_constants::DIR, static_cast<int64_t>(value.dir));
    result.insert(server_constants::HAIRMODEL, static_cast<int64_t>(value.hairModel));
    result.insert(server_constants::SHIRTMODEL, static_cast<int64_t>(value.shirtModel));
    result.insert(server_constants::PANTSMODEL, static_cast<int64_t>(value.pantsModel));
    result.insert(server_constants::HAIRCOLOR, color(value.hairColor));
    result.insert(server_constants::PANTSCOLOR, color(value.pantsColor));
    result.insert(server_constants::CURRENTZONE, static_cast<int64_t>(value.currentZone));
    result.insert(server_constants::DUNGEONLEVEL, static_cast<int64_t>(value.dungeonLevel));
    if (value.interior) {
        result.insert(server_constants::INTERIOR, toml::table{
            {server_constants::INTERIORID, static_cast<int64_t>(value.interior->interior_id)},
            {server_constants::HOUSEINDEX, static_cast<int64_t>(value.interior->house_index)}
        });
    }
    if (value.heldItem) {
        result.insert(server_constants::HELDITEM, static_cast<int64_t>(*value.heldItem));
    }
    toml::table items;
    for (const auto& [slot, item] : value.items) {
        items.insert(std::to_string(slot), stack(item));
    }
    result.insert(server_constants::ITEMS, std::move(items));
    result.insert(server_constants::CURRENTSLOT, static_cast<int64_t>(value.current_slot));
    result.insert(server_constants::INVENTORYCAPACITY, static_cast<int64_t>(value.cap_inventory));
    result.insert(server_constants::GOLD_2, static_cast<int64_t>(value.gold));
    result.insert(server_constants::HEALTH_2, static_cast<int64_t>(value.health));
    result.insert(server_constants::MAXHEALTH, static_cast<int64_t>(value.maxHealth));
    result.insert(server_constants::ENERGY_2, static_cast<int64_t>(value.energy));
    result.insert(server_constants::MAXENERGY, static_cast<int64_t>(value.maxEnergy));
    result.insert(server_constants::ZONEDATA, zone(value.zoneData));
    result.insert(server_constants::COMBATPROGRESS, combatProgress(value.combatProgress));
    toml::array messages;
    for (const ChatMessageData& msg : value.newMessages) {
        messages.push_back(toml::table{{server_constants::SENDER, msg.sender}, {server_constants::TEXT, msg.text}});
    }
    result.insert(server_constants::NEWMESSAGES, std::move(messages));
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
    result.insert(server_constants::CHARACTERNAME, value.characterName);
    result.insert(server_constants::HAIRMODEL, static_cast<int64_t>(value.hairModel));
    result.insert(server_constants::SHIRTMODEL, static_cast<int64_t>(value.shirtModel));
    result.insert(server_constants::PANTSMODEL, static_cast<int64_t>(value.pantsModel));
    result.insert(server_constants::HAIRCOLOR, color(value.hairColor));
    result.insert(server_constants::PANTSCOLOR, color(value.pantsColor));
    return result;
}

toml::table PersistenceSerializer::serialize(const GameIndexDTO& dto) {
    toml::table result;
    result.insert(server_constants::ID, dto.id);
    result.insert(server_constants::FARMNAME, dto.farmName);
    result.insert(server_constants::MAPPATH, dto.mapPath);
    result.insert(server_constants::USERS, strings(dto.users));
    result.insert(server_constants::ISMULTIPLAYER, dto.isMultiplayer);

    toml::table previews;
    for (const auto& [userName, player] : dto.playerPreviews) {
        previews.insert(userName, indexPlayer(player));
    }
    result.insert(server_constants::PLAYERPREVIEWS, std::move(previews));
    return result;
}

toml::table PersistenceSerializer::serialize(const PersistenceDTO& dto) {
    toml::table result;
    result.insert(server_constants::ID, dto.id);
    result.insert(server_constants::FARMNAME, dto.farmName);
    result.insert(server_constants::MAPID, static_cast<int64_t>(dto.mapId));
    result.insert(server_constants::CURRENTDAY, static_cast<int64_t>(dto.currentDay));
    result.insert(server_constants::USERS, strings(dto.users));

    toml::table players;
    for (const auto& [username, data] : dto.players) {
        players.insert(username, player(data));
    }
    result.insert(server_constants::PLAYERS, std::move(players));
    result.insert(server_constants::FARM, farm(dto.farm));
    toml::array interiors;
    for (const auto& room: dto.interiors) interiors.push_back(interior(room));
    result.insert(server_constants::INTERIORS, std::move(interiors));
    toml::table dungeonFurniture;
    for (const auto& [level, items]: dto.dungeonFurniture) {
        toml::array entries;
        for (const auto& item: items) entries.push_back(furniture(item));
        dungeonFurniture.insert(std::to_string(level), std::move(entries));
    }
    result.insert(server_constants::DUNGEONFURNITURE, std::move(dungeonFurniture));
    toml::table dungeonStorageLoot;
    for (const auto& [level, items]: dto.dungeonStorageLoot) {
        toml::array entries;
        for (const auto& item: items) entries.push_back(loot(item));
        dungeonStorageLoot.insert(std::to_string(level), std::move(entries));
    }
    result.insert(server_constants::DUNGEONSTORAGELOOT, std::move(dungeonStorageLoot));

    toml::table chestInventories;
    for (const auto& [id, contents]: dto.chestInventories) {
        toml::table slots;
        for (const auto& [slot, item]: contents) {
            slots.insert(std::to_string(slot), stack(item));
        }
        chestInventories.insert(std::to_string(id), std::move(slots));
    }
    result.insert(server_constants::CHESTINVENTORIES, std::move(chestInventories));
    toml::array emptySoilElapsed;
    for (const auto& [pos, elapsed] : dto.emptySoilElapsed) {
        emptySoilElapsed.push_back(toml::table{{server_constants::POS, position(pos)},
            {server_constants::ELAPSED, checkedInteger(elapsed)}});
    }
    result.insert(server_constants::EMPTYSOILELAPSED, std::move(emptySoilElapsed));
    result.insert(server_constants::CROPGROWTHPROGRESS, uint64Map(dto.cropGrowthProgress));
    result.insert(server_constants::CROPWATERREMAININGTIME, uint64Map(dto.cropWaterRemainingTime));
    result.insert(server_constants::HARVESTEDCROPS, boolMap(dto.harvestedCrops));
    result.insert(server_constants::REGROWINGCROPS, boolMap(dto.regrowingCrops));
    result.insert(server_constants::ANIMALPRODUCTIONPROGRESS, uint64Map(dto.animalProductionProgress));
    result.insert(server_constants::ANIMALAVAILABLEPRODUCTS, uint32Map(dto.animalAvailableProducts));
    result.insert(server_constants::COMMUNITYCENTERACTIONPROGRESS, communityProgress(dto.communityCenterActionProgress));
    return result;
}
