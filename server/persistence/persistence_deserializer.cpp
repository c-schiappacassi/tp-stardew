#include "persistence_deserializer.h"

#include <limits>
#include <set>
#include <stdexcept>
#include <utility>

#include "inventory/inventory.h"

#include "constants.h"

const toml::table& PersistenceDeserializer::requiredTable(const toml::table& table,
                                                          const char* key) {
    const toml::table* result = table[key].as_table();
    if (!result) {
        throw std::runtime_error(std::string("Missing or invalid TOML table: ") + key);
    }
    return *result;
}

const toml::array& PersistenceDeserializer::requiredArray(const toml::table& table,
                                                          const char* key) {
    const toml::array* result = table[key].as_array();
    if (!result) {
        throw std::runtime_error(std::string("Missing or invalid TOML array: ") + key);
    }
    return *result;
}

int64_t PersistenceDeserializer::integer(const toml::table& table, const std::string& key,
                                         int64_t min, int64_t max) {
    auto value = table[key].value<int64_t>();
    if (!value || *value < min || *value > max) {
        throw std::runtime_error("Missing or out-of-range TOML integer: " + key);
    }
    return *value;
}

bool PersistenceDeserializer::boolean(const toml::table& table, const std::string& key) {
    auto value = table[key].value<bool>();
    if (!value) {
        throw std::runtime_error("Missing or invalid TOML boolean: " + key);
    }
    return *value;
}

std::string PersistenceDeserializer::string(const toml::table& table,
                                            const std::string& key) {
    auto value = table[key].value<std::string>();
    if (!value) {
        throw std::runtime_error("Missing or invalid TOML string: " + key);
    }
    return *value;
}

uint32_t PersistenceDeserializer::numericKey(const std::string& key) {
    if (key.empty()) {
        throw std::runtime_error("Empty numeric TOML key");
    }
    for (char digit : key) {
        if (digit < '0' || digit > '9') {
            throw std::runtime_error("Invalid numeric TOML key: " + key);
        }
    }
    try {
        size_t length = 0;
        unsigned long long value = std::stoull(key, &length);
        if (length != key.size() || value > std::numeric_limits<uint32_t>::max()) {
            throw std::runtime_error("Out-of-range numeric TOML key: " + key);
        }
        return static_cast<uint32_t>(value);
    } catch (const std::out_of_range&) {
        throw std::runtime_error("Out-of-range numeric TOML key: " + key);
    }
}

std::map<uint32_t, uint64_t> PersistenceDeserializer::uint64Map(const toml::table& table) {
    std::map<uint32_t, uint64_t> result;
    for (const auto& [key, node] : table) {
        std::string name(key.str());
        result.emplace(numericKey(name), static_cast<uint64_t>(
            integer(table, name, 0, std::numeric_limits<int64_t>::max())));
    }
    return result;
}

std::map<uint32_t, uint32_t> PersistenceDeserializer::uint32Map(const toml::table& table) {
    std::map<uint32_t, uint32_t> result;
    for (const auto& [key, node] : table) {
        std::string name(key.str());
        result.emplace(numericKey(name), static_cast<uint32_t>(
            integer(table, name, 0, std::numeric_limits<uint32_t>::max())));
    }
    return result;
}

std::map<uint32_t, bool> PersistenceDeserializer::boolMap(const toml::table& table) {
    std::map<uint32_t, bool> result;
    for (const auto& [key, node] : table) {
        std::string name(key.str());
        result.emplace(numericKey(name), boolean(table, name));
    }
    return result;
}

Position PersistenceDeserializer::position(const toml::table& table) {
    return Position(static_cast<int>(integer(table, "x", std::numeric_limits<int>::min(),
                                             std::numeric_limits<int>::max())),
                    static_cast<int>(integer(table, "y", std::numeric_limits<int>::min(),
                                             std::numeric_limits<int>::max())));
}

Color PersistenceDeserializer::color(const toml::table& table) {
    Color result;
    result.r = static_cast<uint8_t>(integer(table, "r", 0, UINT8_MAX));
    result.g = static_cast<uint8_t>(integer(table, "g", 0, UINT8_MAX));
    result.b = static_cast<uint8_t>(integer(table, "b", 0, UINT8_MAX));
    return result;
}

Stack PersistenceDeserializer::stack(const toml::table& table) {
    Stack result;
    result.item_id = static_cast<ItemId>(integer(table, "itemId", 0, UINT16_MAX));
    result.quantity = static_cast<uint16_t>(integer(table, "quantity", 0, UINT16_MAX));
    result.qual = static_cast<Quality>(integer(table, "quality", 0, UINT8_MAX));
    return result;
}

FurnitureData PersistenceDeserializer::furniture(const toml::table& table) {
    FurnitureData result;
    result.id = static_cast<uint32_t>(integer(table, "id", 0, UINT32_MAX));
    result.pos = position(requiredTable(table, "pos"));
    result.type = static_cast<FurnitureId>(integer(table, "type", 0, UINT8_MAX));
    result.dir = static_cast<Direction>(integer(table, "dir", 0, UINT8_MAX));
    return result;
}

LootData PersistenceDeserializer::loot(const toml::table& table) {
    LootData result;
    result.id = static_cast<uint32_t>(integer(table, "id", 0, UINT32_MAX));
    result.pos = position(requiredTable(table, "pos"));
    result.item = static_cast<ItemId>(integer(table, "item", 0, UINT16_MAX));
    result.quantity = static_cast<uint32_t>(integer(table, "quantity", 0, UINT32_MAX));
    result.quality = static_cast<Quality>(integer(table, "quality", 0, UINT8_MAX));
    result.kind = static_cast<LootKind>(integer(table, "kind", 0, UINT8_MAX));
    return result;
}

CropData PersistenceDeserializer::crop(const toml::table& table) {
    CropData result;
    result.id = static_cast<uint32_t>(integer(table, "id", 0, UINT32_MAX));
    result.pos = position(requiredTable(table, "pos"));
    result.type = static_cast<CropType>(integer(table, "type", 0, UINT8_MAX));
    result.state = static_cast<CropState>(integer(table, "state", 0, UINT8_MAX));
    result.wattered = boolean(table, "wattered");
    return result;
}

AnimalData PersistenceDeserializer::animal(const toml::table& table) {
    AnimalData result;
    result.id = static_cast<uint32_t>(integer(table, "id", 0, UINT32_MAX));
    result.pos = position(requiredTable(table, "pos"));
    result.dir = static_cast<Direction>(integer(table, "dir", 0, UINT8_MAX));
    result.type = static_cast<AnimalType>(integer(table, "type", 0, UINT8_MAX));
    result.state = static_cast<AnimalState>(integer(table, "state", 0, UINT8_MAX));
    result.needs_interaction = boolean(table, "needsInteraction");
    return result;
}

std::vector<std::string> PersistenceDeserializer::strings(const toml::array& array) {
    std::vector<std::string> result;
    for (const auto& node : array) {
        auto value = node.value<std::string>();
        if (!value) {
            throw std::runtime_error("Expected a string in TOML array");
        }
        result.push_back(*value);
    }
    return result;
}

FarmData PersistenceDeserializer::farm(const toml::table& table) {
    FarmData result;
    for (const auto& node : requiredArray(table, "crops")) {
        if (!node.is_table()) throw std::runtime_error("Invalid crop entry");
        result.crops.push_back(crop(*node.as_table()));
    }
    for (const auto& node : requiredArray(table, "animals")) {
        if (!node.is_table()) throw std::runtime_error("Invalid animal entry");
        result.animals.push_back(animal(*node.as_table()));
    }
    for (const auto& node : requiredArray(table, "tilledSoil")) {
        if (!node.is_table()) throw std::runtime_error("Invalid tilled soil entry");
        result.tilledSoil.push_back(position(*node.as_table()));
    }
    for (const auto& node : requiredArray(table, "furniture")) {
        if (!node.is_table()) throw std::runtime_error("Invalid furniture entry");
        result.furniture.push_back(furniture(*node.as_table()));
    }
    for (const auto& node : requiredArray(table, "loot")) {
        if (!node.is_table()) throw std::runtime_error("Invalid loot entry");
        result.loot.push_back(loot(*node.as_table()));
    }
    if (table.contains("chests")) {
        for (const auto& node: requiredArray(table, "chests")) {
            const auto* chest = node.as_table();
            if (!chest)
                throw std::runtime_error("Invalid chest entry");
            result.furniture.push_back({static_cast<uint32_t>(integer(*chest, "id", 1, UINT32_MAX)), position(requiredTable(*chest, "pos")), FurnitureId::Chest, Direction::Down});
        }
    }
    return result;
}

EnemySnapshot PersistenceDeserializer::enemy(const toml::table& table) {
    EnemySnapshot result;
    result.id = static_cast<uint32_t>(integer(table, "id", 0, UINT32_MAX));
    result.pos = position(requiredTable(table, "pos"));
    result.dir = static_cast<Direction>(integer(table, "dir", 0, UINT8_MAX));
    result.type = static_cast<EnemyType>(integer(table, "type", 0, UINT8_MAX));
    result.state = static_cast<EnemyState>(integer(table, "state", 0, UINT8_MAX));
    result.health = static_cast<uint16_t>(integer(table, "health", 0, UINT16_MAX));
    result.maxHealth = static_cast<uint16_t>(integer(table, "maxHealth", 0, UINT16_MAX));
    result.frenzied = boolean(table, "frenzied");
    result.action = static_cast<uint32_t>(integer(table, "action", 0, UINT32_MAX));
    return result;
}

ProjectileData PersistenceDeserializer::projectile(const toml::table& table) {
    ProjectileData result;
    result.id = static_cast<uint32_t>(integer(table, "id", 0, UINT32_MAX));
    result.pos = position(requiredTable(table, "pos"));
    result.type = static_cast<FlameType>(integer(table, "type", 0, UINT8_MAX));
    return result;
}

TrapData PersistenceDeserializer::trap(const toml::table& table) {
    TrapData result;
    result.id = static_cast<uint32_t>(integer(table, "id", 0, UINT32_MAX));
    result.pos = position(requiredTable(table, "pos"));
    result.dir = static_cast<Direction>(integer(table, "dir", 0, UINT8_MAX));
    result.type = static_cast<TrapType>(integer(table, "type", 0, UINT8_MAX));
    result.state = static_cast<TrapState>(integer(table, "state", 0, UINT8_MAX));
    result.range = static_cast<uint16_t>(integer(table, "range", 0, UINT16_MAX));
    return result;
}

DungeonCombatData PersistenceDeserializer::combat(const toml::table& table) {
    DungeonCombatData result;
    result.visitId = static_cast<uint32_t>(integer(table, "visitId", 0, UINT32_MAX));
    result.room = static_cast<uint8_t>(integer(table, "room", 0, UINT8_MAX));
    for (const auto& node : requiredArray(table, "enemies")) {
        if (!node.is_table()) throw std::runtime_error("Invalid enemy entry");
        result.enemies.push_back(enemy(*node.as_table()));
    }
    for (const auto& node : requiredArray(table, "projectiles")) {
        if (!node.is_table()) throw std::runtime_error("Invalid projectile entry");
        result.projectiles.push_back(projectile(*node.as_table()));
    }
    for (const auto& node : requiredArray(table, "traps")) {
        if (!node.is_table()) throw std::runtime_error("Invalid trap entry");
        result.traps.push_back(trap(*node.as_table()));
    }
    for (const auto& node : requiredArray(table, "loot")) {
        if (!node.is_table()) throw std::runtime_error("Invalid combat loot entry");
        result.loot.push_back(loot(*node.as_table()));
    }
    return result;
}

MineralData PersistenceDeserializer::mineral(const toml::table& table) {
    MineralData result;
    result.id = static_cast<uint32_t>(integer(table, "id", 0, UINT32_MAX));
    result.pos = position(requiredTable(table, "pos"));
    result.type = static_cast<ResourceType>(integer(table, "type", 0, UINT8_MAX));
    result.state = static_cast<MineralState>(integer(table, "state", 0, UINT8_MAX));
    result.health = table.contains("health") ? static_cast<uint16_t>(integer(table, "health", 0, UINT16_MAX))
                                             : static_cast<uint16_t>(result.state == MineralState::Broken ? 0 : 1);
    return result;
}

LevelData PersistenceDeserializer::level(const toml::table& table) {
    LevelData result;
    result.entry = position(requiredTable(table, "entry"));
    result.exit = position(requiredTable(table, "exit"));
    for (const auto& node : requiredArray(table, "minerals")) {
        if (!node.is_table()) throw std::runtime_error("Invalid mineral entry");
        result.minerals.push_back(mineral(*node.as_table()));
    }
    result.combat = combat(requiredTable(table, "combat"));
    return result;
}

MineData PersistenceDeserializer::mine(const toml::table& table) {
    MineData result;
    result.current_level = static_cast<uint8_t>(integer(table, "currentLevel", 0, UINT8_MAX));
    result.current_level_data = level(requiredTable(table, "currentLevelData"));
    return result;
}

InteriorData PersistenceDeserializer::interior(const toml::table& table) {
    InteriorData result;
    result.interior_id = static_cast<InteriorId>(integer(table, "interiorId", 0, UINT8_MAX));
    result.house_index = static_cast<uint8_t>(integer(table, "houseIndex", 0, UINT8_MAX));
    for (const auto& node : requiredArray(table, "furniture")) {
        if (!node.is_table()) throw std::runtime_error("Invalid interior furniture entry");
        result.furniture.push_back(furniture(*node.as_table()));
    }
    if (table.contains("loot")) {
        for (const auto& node: requiredArray(table, "loot")) {
            if (!node.is_table())
                throw std::runtime_error("Invalid interior loot");
            result.loot.push_back(loot(*node.as_table()));
        }
    }
    return result;
}

ZoneData PersistenceDeserializer::zone(const toml::table& table) {
    std::string type = string(table, "type");
    if (type == "farm") return farm(requiredTable(table, "data"));
    if (type == "mine") return mine(requiredTable(table, "data"));
    if (type == "interior") return interior(requiredTable(table, "data"));
    if (type == "none") return std::monostate{};
    throw std::runtime_error("Unknown zone type: " + type);
}

CombatProgressData PersistenceDeserializer::combatProgress(const toml::table& table) {
    CombatProgressData result;
    result.skeletons = boolean(table, "skeletons");
    result.vampires = boolean(table, "vampires");
    result.orcs = boolean(table, "orcs");
    result.skull = boolean(table, "skull");
    return result;
}

PlayerDataLocal PersistenceDeserializer::player(const toml::table& table) {
    PlayerDataLocal result;
    result.characterName = string(table, "characterName");
    result.pos = position(requiredTable(table, "pos"));
    result.state = static_cast<PlayerState>(integer(table, "state", 0, UINT8_MAX));
    result.dir = static_cast<Direction>(integer(table, "dir", 0, UINT8_MAX));
    result.hairModel = static_cast<uint16_t>(integer(table, "hairModel", 0, UINT16_MAX));
    result.shirtModel = static_cast<uint16_t>(integer(table, "shirtModel", 0, UINT16_MAX));
    result.pantsModel = static_cast<uint16_t>(integer(table, "pantsModel", 0, UINT16_MAX));
    result.hairColor = color(requiredTable(table, "hairColor"));
    result.pantsColor = color(requiredTable(table, "pantsColor"));
    result.currentZone = static_cast<Zone>(integer(table, "currentZone", 0, UINT8_MAX));
    if (table.contains("interior")) {
        const toml::table& location = requiredTable(table, "interior");
        InteriorLocation value;
        value.interior_id = static_cast<InteriorId>(integer(location, "interiorId", 0, UINT8_MAX));
        value.house_index = static_cast<uint8_t>(integer(location, "houseIndex", 0, UINT8_MAX));
        result.interior = value;
    }
    if (table.contains("heldItem")) {
        result.heldItem = static_cast<ItemId>(integer(table, "heldItem", 0, UINT16_MAX));
    }
    const toml::table& items = requiredTable(table, "items");
    for (const auto& [key, node] : items) {
        if (!node.is_table()) throw std::runtime_error("Invalid inventory stack");
        uint32_t slot = numericKey(std::string(key.str()));
        if (slot > UINT8_MAX) throw std::runtime_error("Inventory slot out of range");
        result.items.emplace(static_cast<uint8_t>(slot), stack(*node.as_table()));
    }
    result.current_slot = static_cast<uint8_t>(integer(table, "currentSlot", 0, UINT8_MAX));
    result.cap_inventory = table.contains("inventoryCapacity") ?
        static_cast<uint8_t>(integer(table, "inventoryCapacity", 1, UINT8_MAX)) : PLAYER_INVENTORY_CAPACITY;
    if (result.current_slot >= result.cap_inventory ||
        (!result.items.empty() && result.items.rbegin()->first >= result.cap_inventory)) {
        throw std::runtime_error("Inventory slot exceeds saved capacity");
    }
    result.gold = static_cast<uint32_t>(integer(table, "gold", 0, UINT32_MAX));
    result.health = static_cast<uint16_t>(integer(table, "health", 0, UINT16_MAX));
    result.maxHealth = static_cast<uint16_t>(integer(table, "maxHealth", 0, UINT16_MAX));
    result.energy = static_cast<uint16_t>(integer(table, "energy", 0, UINT16_MAX));
    result.maxEnergy = static_cast<uint16_t>(integer(table, "maxEnergy", 0, UINT16_MAX));
    result.zoneData = zone(requiredTable(table, "zoneData"));
    if (table.contains("dungeonLevel")) {
        result.dungeonLevel = static_cast<uint8_t>(integer(table, "dungeonLevel", 0, UINT8_MAX));
    } else if (const auto* mineData = std::get_if<MineData>(&result.zoneData)) {
        result.dungeonLevel = mineData->current_level;
    }
    result.combatProgress = combatProgress(requiredTable(table, "combatProgress"));
    for (const auto& node : requiredArray(table, "newMessages")) {
        if (!node.is_table()) throw std::runtime_error("Invalid chat message");
        const toml::table& msg = *node.as_table();
        result.newMessages.push_back(ChatMessageData{string(msg, "sender"), string(msg, "text")});
    }
    return result;
}

CommunitySaveData PersistenceDeserializer::communityProgress(const toml::table& table) {
    CommunitySaveData result;
    for (const auto& [task, actions] : table) {
        if (!actions.is_table()) throw std::runtime_error("Invalid community center task");
        std::string taskName(task.str());
        for (const auto& [action, counters] : *actions.as_table()) {
            if (!counters.is_table()) throw std::runtime_error("Invalid community center action");
            std::string actionName(action.str());
            for (const auto& [name, count] : *counters.as_table()) {
                std::string counterName(name.str());
                result[taskName][actionName][counterName] = static_cast<uint32_t>(
                    integer(*counters.as_table(), counterName, 0, UINT32_MAX));
            }
            if (counters.as_table()->empty()) result[taskName][actionName] = {};
        }
        if (actions.as_table()->empty()) result[taskName] = {};
    }
    return result;
}

GameIndexPlayerDTO PersistenceDeserializer::indexPlayer(const toml::table& table) {
    GameIndexPlayerDTO result;
    result.characterName = string(table, "characterName");
    result.hairModel = static_cast<uint16_t>(integer(table, "hairModel", 0, UINT16_MAX));
    result.shirtModel = static_cast<uint16_t>(integer(table, "shirtModel", 0, UINT16_MAX));
    result.pantsModel = static_cast<uint16_t>(integer(table, "pantsModel", 0, UINT16_MAX));
    result.hairColor = color(requiredTable(table, "hairColor"));
    result.pantsColor = color(requiredTable(table, "pantsColor"));
    return result;
}

GameIndexDTO PersistenceDeserializer::deserializeIndex(const toml::table& table) {
    GameIndexDTO result;
    result.id = string(table, "id");
    result.farmName = string(table, "farmName");
    result.mapPath = table["mapPath"].value<std::string>().value_or("");
    result.users = strings(requiredArray(table, "users"));
    result.isMultiplayer = boolean(table, "isMultiplayer");
    const toml::table& previews = requiredTable(table, "playerPreviews");
    for (const auto& [userName, node] : previews) {
        if (!node.is_table()) {
            throw std::runtime_error("Invalid player preview in persistence index");
        }
        result.playerPreviews.emplace(std::string(userName.str()), indexPlayer(*node.as_table()));
    }
    for (const std::string& userName : result.users) {
        if (result.playerPreviews.find(userName) == result.playerPreviews.end()) {
            throw std::runtime_error("Missing player preview in index for: " + userName);
        }
    }
    return result;
}

PersistenceDTO PersistenceDeserializer::deserializeGame(const toml::table& table) {
    PersistenceDTO result;
    result.id = string(table, "id");
    result.farmName = string(table, "farmName");
    result.mapId = static_cast<uint8_t>(integer(table, "mapId", 0, UINT8_MAX));
    result.currentDay = static_cast<uint32_t>(integer(table, "currentDay", 0, UINT32_MAX));
    result.users = strings(requiredArray(table, "users"));
    const toml::table& players = requiredTable(table, "players");
    for (const auto& [username, data] : players) {
        if (!data.is_table()) throw std::runtime_error("Invalid player entry");
        result.players.emplace(std::string(username.str()), player(*data.as_table()));
    }
    result.farm = farm(requiredTable(table, "farm"));
    if (table.contains("interiors")) {
        std::set<std::pair<InteriorId, uint8_t>> rooms;
        for (const auto& node: requiredArray(table, "interiors")) {
            if (!node.is_table())
                throw std::runtime_error("Invalid saved interior");
            auto room = interior(*node.as_table());
            if (room.interior_id != InteriorId::PlayerHouse || room.house_index >= 4 || !rooms.emplace(room.interior_id, room.house_index).second)
                throw std::runtime_error("Invalid or duplicate saved house");
            result.interiors.push_back(std::move(room));
        }
    }
    if (table.contains("dungeonFurniture")) {
        for (const auto& [key, node]: requiredTable(table, "dungeonFurniture")) {
            const auto level = numericKey(std::string(key.str()));
            if (level == 0 || level > 10 || !node.is_array() || result.dungeonFurniture.contains(static_cast<uint8_t>(level)))
                throw std::runtime_error("Invalid or duplicate saved dungeon level");
            auto& items = result.dungeonFurniture[static_cast<uint8_t>(level)];
            for (const auto& entry: *node.as_array()) {
                if (!entry.is_table())
                    throw std::runtime_error("Invalid dungeon furniture");
                auto item = furniture(*entry.as_table());
                if (item.type != FurnitureId::Chest)
                    throw std::runtime_error("Unsupported dungeon furniture");
                items.push_back(item);
            }
        }
    }

    if (table.contains("dungeonStorageLoot")) {
        for (const auto& [key, node]: requiredTable(table, "dungeonStorageLoot")) {
            const auto level = numericKey(std::string(key.str()));
            if (level == 0 || level > 10 || !node.is_array() || !result.dungeonFurniture.contains(static_cast<uint8_t>(level)) ||
                result.dungeonStorageLoot.contains(static_cast<uint8_t>(level)))
                throw std::runtime_error("Invalid saved dungeon storage loot");
            auto& items = result.dungeonStorageLoot[static_cast<uint8_t>(level)];
            for (const auto& entry: *node.as_array()) {
                if (!entry.is_table())
                    throw std::runtime_error("Invalid dungeon loot");
                items.push_back(loot(*entry.as_table()));
            }
        }
    }
    if (table.contains("chestInventories")) {
        for (const auto& [key, node]: requiredTable(table, "chestInventories")) {
            const auto id = numericKey(std::string(key.str()));
            const auto* items = node.as_table();
            if (id == 0 || !items)
                throw std::runtime_error("Invalid chest inventory");
            std::map<uint8_t, Stack> contents;
            for (const auto& [slotKey, item]: *items) {
                const auto slot = numericKey(std::string(slotKey.str()));
                if (slot >= CHEST_INVENTORY_CAPACITY || !item.is_table() || !contents.emplace(static_cast<uint8_t>(slot), stack(*item.as_table())).second) {
                    throw std::runtime_error("Invalid chest inventory slot");
                }
            }
            Inventory{CHEST_INVENTORY_CAPACITY}.restoreSlots(contents);
            if (!result.chestInventories.emplace(id, std::move(contents)).second) {
                throw std::runtime_error("Duplicate chest inventory");
            }
        }
    }
    std::set<uint32_t> chestIds;
    const auto validateChests = [&](const std::vector<FurnitureData>& items) {
        std::set<uint32_t> ids;
        for (const auto& item: items) {
            if (item.id == 0 || item.type > FurnitureId::Chest || item.dir > Direction::Right || !ids.insert(item.id).second)
                throw std::runtime_error("Invalid or duplicate furniture");
            if (item.type == FurnitureId::Chest && (!chestIds.insert(item.id).second || !result.chestInventories.contains(item.id)))
                throw std::runtime_error("Duplicate chest or missing contents");
        }
    };
    validateChests(result.farm.furniture);
    for (const auto& room: result.interiors) validateChests(room.furniture);
    for (const auto& [level, items]: result.dungeonFurniture) validateChests(items);
    if (result.chestInventories.size() != chestIds.size())
        throw std::runtime_error("Missing or orphaned chest inventories");
    if (table.contains("emptySoilElapsed")) {
        for (const auto& entry : requiredArray(table, "emptySoilElapsed")) {
            const auto* soil = entry.as_table();
            if (!soil) throw std::runtime_error("Invalid empty soil progress entry");
            result.emptySoilElapsed.emplace_back(position(requiredTable(*soil, "pos")),
                static_cast<uint64_t>(integer(*soil, "elapsed", 0,
                    std::numeric_limits<int64_t>::max())));
        }
    }
    result.cropGrowthProgress = uint64Map(requiredTable(table, "cropGrowthProgress"));
    result.cropWaterRemainingTime = uint64Map(requiredTable(table, "cropWaterRemainingTime"));
    result.harvestedCrops = boolMap(requiredTable(table, "harvestedCrops"));
    if (table.contains("regrowingCrops")) {
        result.regrowingCrops = boolMap(requiredTable(table, "regrowingCrops"));
    }
    result.animalProductionProgress = uint64Map(requiredTable(table, "animalProductionProgress"));
    result.animalAvailableProducts = uint32Map(requiredTable(table, "animalAvailableProducts"));
    result.communityCenterActionProgress = communityProgress(
        requiredTable(table, "communityCenterActionProgress"));
    return result;
}
