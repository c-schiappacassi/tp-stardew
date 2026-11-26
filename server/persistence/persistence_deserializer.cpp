#include "persistence_deserializer.h"

#include "server_constants.h"

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
        throw std::runtime_error(std::string(server_constants::MISSING_OR_INVALID_TOML_TABLE_PREFIX) + key);
    }
    return *result;
}

const toml::array& PersistenceDeserializer::requiredArray(const toml::table& table,
                                                          const char* key) {
    const toml::array* result = table[key].as_array();
    if (!result) {
        throw std::runtime_error(std::string(server_constants::MISSING_OR_INVALID_TOML_ARRAY_PREFIX) + key);
    }
    return *result;
}

int64_t PersistenceDeserializer::integer(const toml::table& table, const std::string& key,
                                         int64_t min, int64_t max) {
    auto value = table[key].value<int64_t>();
    if (!value || *value < min || *value > max) {
        throw std::runtime_error(server_constants::MISSING_OR_OUT_OF_RANGE_TOML_INTEGER_PREFIX + key);
    }
    return *value;
}

bool PersistenceDeserializer::boolean(const toml::table& table, const std::string& key) {
    auto value = table[key].value<bool>();
    if (!value) {
        throw std::runtime_error(server_constants::MISSING_OR_INVALID_TOML_BOOLEAN_PREFIX + key);
    }
    return *value;
}

std::string PersistenceDeserializer::string(const toml::table& table,
                                            const std::string& key) {
    auto value = table[key].value<std::string>();
    if (!value) {
        throw std::runtime_error(server_constants::MISSING_OR_INVALID_TOML_STRING_PREFIX + key);
    }
    return *value;
}

uint32_t PersistenceDeserializer::numericKey(const std::string& key) {
    if (key.empty()) {
        throw std::runtime_error(server_constants::EMPTY_NUMERIC_TOML_KEY);
    }
    for (char digit : key) {
        if (digit < '0' || digit > '9') {
            throw std::runtime_error(server_constants::INVALID_NUMERIC_TOML_KEY_PREFIX + key);
        }
    }
    try {
        size_t length = 0;
        unsigned long long value = std::stoull(key, &length);
        if (length != key.size() || value > std::numeric_limits<uint32_t>::max()) {
            throw std::runtime_error(server_constants::OUT_OF_RANGE_NUMERIC_TOML_KEY_PREFIX + key);
        }
        return static_cast<uint32_t>(value);
    } catch (const std::out_of_range&) {
        throw std::runtime_error(server_constants::OUT_OF_RANGE_NUMERIC_TOML_KEY_PREFIX + key);
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
    return Position(static_cast<int>(integer(table, server_constants::X, std::numeric_limits<int>::min(),
                                             std::numeric_limits<int>::max())),
                    static_cast<int>(integer(table, server_constants::Y, std::numeric_limits<int>::min(),
                                             std::numeric_limits<int>::max())));
}

Color PersistenceDeserializer::color(const toml::table& table) {
    Color result;
    result.r = static_cast<uint8_t>(integer(table, server_constants::R, 0, UINT8_MAX));
    result.g = static_cast<uint8_t>(integer(table, server_constants::G, 0, UINT8_MAX));
    result.b = static_cast<uint8_t>(integer(table, server_constants::B, 0, UINT8_MAX));
    return result;
}

Stack PersistenceDeserializer::stack(const toml::table& table) {
    Stack result;
    result.item_id = static_cast<ItemId>(integer(table, server_constants::ITEMID, 0, UINT16_MAX));
    result.quantity = static_cast<uint16_t>(integer(table, server_constants::QUANTITY, 0, UINT16_MAX));
    result.qual = static_cast<Quality>(integer(table, server_constants::QUALITY, 0, UINT8_MAX));
    return result;
}

FurnitureData PersistenceDeserializer::furniture(const toml::table& table) {
    FurnitureData result;
    result.id = static_cast<uint32_t>(integer(table, server_constants::ID, 0, UINT32_MAX));
    result.pos = position(requiredTable(table, server_constants::POS));
    result.type = static_cast<FurnitureId>(integer(table, server_constants::TYPE, 0, UINT8_MAX));
    result.dir = static_cast<Direction>(integer(table, server_constants::DIR, 0, UINT8_MAX));
    return result;
}

LootData PersistenceDeserializer::loot(const toml::table& table) {
    LootData result;
    result.id = static_cast<uint32_t>(integer(table, server_constants::ID, 0, UINT32_MAX));
    result.pos = position(requiredTable(table, server_constants::POS));
    result.item = static_cast<ItemId>(integer(table, server_constants::ITEM, 0, UINT16_MAX));
    result.quantity = static_cast<uint32_t>(integer(table, server_constants::QUANTITY, 0, UINT32_MAX));
    result.quality = static_cast<Quality>(integer(table, server_constants::QUALITY, 0, UINT8_MAX));
    result.kind = static_cast<LootKind>(integer(table, server_constants::KIND, 0, UINT8_MAX));
    return result;
}

CropData PersistenceDeserializer::crop(const toml::table& table) {
    CropData result;
    result.id = static_cast<uint32_t>(integer(table, server_constants::ID, 0, UINT32_MAX));
    result.pos = position(requiredTable(table, server_constants::POS));
    result.type = static_cast<CropType>(integer(table, server_constants::TYPE, 0, UINT8_MAX));
    result.state = static_cast<CropState>(integer(table, server_constants::STATE, 0, UINT8_MAX));
    result.wattered = boolean(table, server_constants::WATTERED);
    return result;
}

AnimalData PersistenceDeserializer::animal(const toml::table& table) {
    AnimalData result;
    result.id = static_cast<uint32_t>(integer(table, server_constants::ID, 0, UINT32_MAX));
    result.pos = position(requiredTable(table, server_constants::POS));
    result.dir = static_cast<Direction>(integer(table, server_constants::DIR, 0, UINT8_MAX));
    result.type = static_cast<AnimalType>(integer(table, server_constants::TYPE, 0, UINT8_MAX));
    result.state = static_cast<AnimalState>(integer(table, server_constants::STATE, 0, UINT8_MAX));
    result.needs_interaction = boolean(table, server_constants::NEEDSINTERACTION);
    return result;
}

std::vector<std::string> PersistenceDeserializer::strings(const toml::array& array) {
    std::vector<std::string> result;
    for (const auto& node : array) {
        auto value = node.value<std::string>();
        if (!value) {
            throw std::runtime_error(server_constants::EXPECTED_A_STRING_IN_TOML_ARRAY);
        }
        result.push_back(*value);
    }
    return result;
}

FarmData PersistenceDeserializer::farm(const toml::table& table) {
    FarmData result;
    for (const auto& node : requiredArray(table, server_constants::CROPS)) {
        if (!node.is_table()) throw std::runtime_error(server_constants::INVALID_CROP_ENTRY);
        result.crops.push_back(crop(*node.as_table()));
    }
    for (const auto& node : requiredArray(table, server_constants::ANIMALS)) {
        if (!node.is_table()) throw std::runtime_error(server_constants::INVALID_ANIMAL_ENTRY);
        result.animals.push_back(animal(*node.as_table()));
    }
    for (const auto& node : requiredArray(table, server_constants::TILLEDSOIL)) {
        if (!node.is_table()) throw std::runtime_error(server_constants::INVALID_TILLED_SOIL_ENTRY);
        result.tilledSoil.push_back(position(*node.as_table()));
    }
    for (const auto& node : requiredArray(table, server_constants::FURNITURE)) {
        if (!node.is_table()) throw std::runtime_error(server_constants::INVALID_FURNITURE_ENTRY);
        result.furniture.push_back(furniture(*node.as_table()));
    }
    for (const auto& node : requiredArray(table, server_constants::LOOT)) {
        if (!node.is_table()) throw std::runtime_error(server_constants::INVALID_LOOT_ENTRY);
        result.loot.push_back(loot(*node.as_table()));
    }
    if (table.contains(server_constants::CHESTS)) {
        for (const auto& node: requiredArray(table, server_constants::CHESTS)) {
            const auto* chest = node.as_table();
            if (!chest)
                throw std::runtime_error(server_constants::INVALID_CHEST_ENTRY);
            result.furniture.push_back({static_cast<uint32_t>(integer(*chest, server_constants::ID, 1, UINT32_MAX)), position(requiredTable(*chest, server_constants::POS)), FurnitureId::Chest, Direction::Down});
        }
    }
    return result;
}

EnemySnapshot PersistenceDeserializer::enemy(const toml::table& table) {
    EnemySnapshot result;
    result.id = static_cast<uint32_t>(integer(table, server_constants::ID, 0, UINT32_MAX));
    result.pos = position(requiredTable(table, server_constants::POS));
    result.dir = static_cast<Direction>(integer(table, server_constants::DIR, 0, UINT8_MAX));
    result.type = static_cast<EnemyType>(integer(table, server_constants::TYPE, 0, UINT8_MAX));
    result.state = static_cast<EnemyState>(integer(table, server_constants::STATE, 0, UINT8_MAX));
    result.health = static_cast<uint16_t>(integer(table, server_constants::HEALTH_2, 0, UINT16_MAX));
    result.maxHealth = static_cast<uint16_t>(integer(table, server_constants::MAXHEALTH, 0, UINT16_MAX));
    result.frenzied = boolean(table, server_constants::FRENZIED);
    result.action = static_cast<uint32_t>(integer(table, server_constants::ACTION, 0, UINT32_MAX));
    return result;
}

ProjectileData PersistenceDeserializer::projectile(const toml::table& table) {
    ProjectileData result;
    result.id = static_cast<uint32_t>(integer(table, server_constants::ID, 0, UINT32_MAX));
    result.pos = position(requiredTable(table, server_constants::POS));
    result.type = static_cast<FlameType>(integer(table, server_constants::TYPE, 0, UINT8_MAX));
    return result;
}

TrapData PersistenceDeserializer::trap(const toml::table& table) {
    TrapData result;
    result.id = static_cast<uint32_t>(integer(table, server_constants::ID, 0, UINT32_MAX));
    result.pos = position(requiredTable(table, server_constants::POS));
    result.dir = static_cast<Direction>(integer(table, server_constants::DIR, 0, UINT8_MAX));
    result.type = static_cast<TrapType>(integer(table, server_constants::TYPE, 0, UINT8_MAX));
    result.state = static_cast<TrapState>(integer(table, server_constants::STATE, 0, UINT8_MAX));
    result.range = static_cast<uint16_t>(integer(table, server_constants::RANGE, 0, UINT16_MAX));
    return result;
}

DungeonCombatData PersistenceDeserializer::combat(const toml::table& table) {
    DungeonCombatData result;
    result.visitId = static_cast<uint32_t>(integer(table, server_constants::VISITID, 0, UINT32_MAX));
    result.room = static_cast<uint8_t>(integer(table, server_constants::ROOM, 0, UINT8_MAX));
    for (const auto& node : requiredArray(table, server_constants::ENEMIES)) {
        if (!node.is_table()) throw std::runtime_error(server_constants::INVALID_ENEMY_ENTRY);
        result.enemies.push_back(enemy(*node.as_table()));
    }
    for (const auto& node : requiredArray(table, server_constants::PROJECTILES)) {
        if (!node.is_table()) throw std::runtime_error(server_constants::INVALID_PROJECTILE_ENTRY);
        result.projectiles.push_back(projectile(*node.as_table()));
    }
    for (const auto& node : requiredArray(table, server_constants::TRAPS)) {
        if (!node.is_table()) throw std::runtime_error(server_constants::INVALID_TRAP_ENTRY);
        result.traps.push_back(trap(*node.as_table()));
    }
    for (const auto& node : requiredArray(table, server_constants::LOOT)) {
        if (!node.is_table()) throw std::runtime_error(server_constants::INVALID_COMBAT_LOOT_ENTRY);
        result.loot.push_back(loot(*node.as_table()));
    }
    return result;
}

MineralData PersistenceDeserializer::mineral(const toml::table& table) {
    MineralData result;
    result.id = static_cast<uint32_t>(integer(table, server_constants::ID, 0, UINT32_MAX));
    result.pos = position(requiredTable(table, server_constants::POS));
    result.type = static_cast<ResourceType>(integer(table, server_constants::TYPE, 0, UINT8_MAX));
    result.state = static_cast<MineralState>(integer(table, server_constants::STATE, 0, UINT8_MAX));
    result.health = table.contains(server_constants::HEALTH_2) ? static_cast<uint16_t>(integer(table, server_constants::HEALTH_2, 0, UINT16_MAX))
                                             : static_cast<uint16_t>(result.state == MineralState::Broken ? 0 : 1);
    return result;
}

LevelData PersistenceDeserializer::level(const toml::table& table) {
    LevelData result;
    result.entry = position(requiredTable(table, server_constants::ENTRY));
    result.exit = position(requiredTable(table, server_constants::EXIT));
    for (const auto& node : requiredArray(table, server_constants::MINERALS)) {
        if (!node.is_table()) throw std::runtime_error(server_constants::INVALID_MINERAL_ENTRY);
        result.minerals.push_back(mineral(*node.as_table()));
    }
    result.combat = combat(requiredTable(table, server_constants::COMBAT));
    return result;
}

MineData PersistenceDeserializer::mine(const toml::table& table) {
    MineData result;
    result.current_level = static_cast<uint8_t>(integer(table, server_constants::CURRENTLEVEL, 0, UINT8_MAX));
    result.current_level_data = level(requiredTable(table, server_constants::CURRENTLEVELDATA));
    return result;
}

InteriorData PersistenceDeserializer::interior(const toml::table& table) {
    InteriorData result;
    result.interior_id = static_cast<InteriorId>(integer(table, server_constants::INTERIORID, 0, UINT8_MAX));
    result.house_index = static_cast<uint8_t>(integer(table, server_constants::HOUSEINDEX, 0, UINT8_MAX));
    for (const auto& node : requiredArray(table, server_constants::FURNITURE)) {
        if (!node.is_table()) throw std::runtime_error(server_constants::INVALID_INTERIOR_FURNITURE_ENTRY);
        result.furniture.push_back(furniture(*node.as_table()));
    }
    if (table.contains(server_constants::LOOT)) {
        for (const auto& node: requiredArray(table, server_constants::LOOT)) {
            if (!node.is_table())
                throw std::runtime_error(server_constants::INVALID_INTERIOR_LOOT);
            result.loot.push_back(loot(*node.as_table()));
        }
    }
    return result;
}

ZoneData PersistenceDeserializer::zone(const toml::table& table) {
    std::string type = string(table, server_constants::TYPE);
    if (type == server_constants::FARM) return farm(requiredTable(table, server_constants::DATA));
    if (type == server_constants::MINE) return mine(requiredTable(table, server_constants::DATA));
    if (type == server_constants::INTERIOR) return interior(requiredTable(table, server_constants::DATA));
    if (type == server_constants::NONE_2) return std::monostate{};
    throw std::runtime_error(server_constants::UNKNOWN_ZONE_TYPE_PREFIX + type);
}

CombatProgressData PersistenceDeserializer::combatProgress(const toml::table& table) {
    CombatProgressData result;
    result.skeletons = boolean(table, server_constants::SKELETONS);
    result.vampires = boolean(table, server_constants::VAMPIRES);
    result.orcs = boolean(table, server_constants::ORCS);
    result.skull = boolean(table, server_constants::SKULL);
    return result;
}

PlayerDataLocal PersistenceDeserializer::player(const toml::table& table) {
    PlayerDataLocal result;
    result.characterName = string(table, server_constants::CHARACTERNAME);
    result.pos = position(requiredTable(table, server_constants::POS));
    result.state = static_cast<PlayerState>(integer(table, server_constants::STATE, 0, UINT8_MAX));
    result.dir = static_cast<Direction>(integer(table, server_constants::DIR, 0, UINT8_MAX));
    result.hairModel = static_cast<uint16_t>(integer(table, server_constants::HAIRMODEL, 0, UINT16_MAX));
    result.shirtModel = static_cast<uint16_t>(integer(table, server_constants::SHIRTMODEL, 0, UINT16_MAX));
    result.pantsModel = static_cast<uint16_t>(integer(table, server_constants::PANTSMODEL, 0, UINT16_MAX));
    result.hairColor = color(requiredTable(table, server_constants::HAIRCOLOR));
    result.pantsColor = color(requiredTable(table, server_constants::PANTSCOLOR));
    result.currentZone = static_cast<Zone>(integer(table, server_constants::CURRENTZONE, 0, UINT8_MAX));
    if (table.contains(server_constants::INTERIOR)) {
        const toml::table& location = requiredTable(table, server_constants::INTERIOR);
        InteriorLocation value;
        value.interior_id = static_cast<InteriorId>(integer(location, server_constants::INTERIORID, 0, UINT8_MAX));
        value.house_index = static_cast<uint8_t>(integer(location, server_constants::HOUSEINDEX, 0, UINT8_MAX));
        result.interior = value;
    }
    if (table.contains(server_constants::HELDITEM)) {
        result.heldItem = static_cast<ItemId>(integer(table, server_constants::HELDITEM, 0, UINT16_MAX));
    }
    const toml::table& items = requiredTable(table, server_constants::ITEMS);
    for (const auto& [key, node] : items) {
        if (!node.is_table()) throw std::runtime_error(server_constants::INVALID_INVENTORY_STACK);
        uint32_t slot = numericKey(std::string(key.str()));
        if (slot > UINT8_MAX) throw std::runtime_error(server_constants::INVENTORY_SLOT_OUT_OF_RANGE);
        result.items.emplace(static_cast<uint8_t>(slot), stack(*node.as_table()));
    }
    result.current_slot = static_cast<uint8_t>(integer(table, server_constants::CURRENTSLOT, 0, UINT8_MAX));
    result.cap_inventory = table.contains(server_constants::INVENTORYCAPACITY) ?
        static_cast<uint8_t>(integer(table, server_constants::INVENTORYCAPACITY, 1, UINT8_MAX)) : PLAYER_INVENTORY_CAPACITY;
    if (result.current_slot >= result.cap_inventory ||
        (!result.items.empty() && result.items.rbegin()->first >= result.cap_inventory)) {
        throw std::runtime_error(server_constants::INVENTORY_SLOT_EXCEEDS_SAVED_CAPACITY);
    }
    result.gold = static_cast<uint32_t>(integer(table, server_constants::GOLD_2, 0, UINT32_MAX));
    result.health = static_cast<uint16_t>(integer(table, server_constants::HEALTH_2, 0, UINT16_MAX));
    result.maxHealth = static_cast<uint16_t>(integer(table, server_constants::MAXHEALTH, 0, UINT16_MAX));
    result.energy = static_cast<uint16_t>(integer(table, server_constants::ENERGY_2, 0, UINT16_MAX));
    result.maxEnergy = static_cast<uint16_t>(integer(table, server_constants::MAXENERGY, 0, UINT16_MAX));
    result.zoneData = zone(requiredTable(table, server_constants::ZONEDATA));
    if (table.contains(server_constants::DUNGEONLEVEL)) {
        result.dungeonLevel = static_cast<uint8_t>(integer(table, server_constants::DUNGEONLEVEL, 0, UINT8_MAX));
    } else if (const auto* mineData = std::get_if<MineData>(&result.zoneData)) {
        result.dungeonLevel = mineData->current_level;
    }
    result.combatProgress = combatProgress(requiredTable(table, server_constants::COMBATPROGRESS));
    for (const auto& node : requiredArray(table, server_constants::NEWMESSAGES)) {
        if (!node.is_table()) throw std::runtime_error(server_constants::INVALID_CHAT_MESSAGE);
        const toml::table& msg = *node.as_table();
        result.newMessages.push_back(ChatMessageData{string(msg, server_constants::SENDER), string(msg, server_constants::TEXT)});
    }
    return result;
}

CommunitySaveData PersistenceDeserializer::communityProgress(const toml::table& table) {
    CommunitySaveData result;
    for (const auto& [task, actions] : table) {
        if (!actions.is_table()) throw std::runtime_error(server_constants::INVALID_COMMUNITY_CENTER_TASK);
        std::string taskName(task.str());
        for (const auto& [action, counters] : *actions.as_table()) {
            if (!counters.is_table()) throw std::runtime_error(server_constants::INVALID_COMMUNITY_CENTER_ACTION);
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
    if (table.contains(server_constants::INITIAL_PLAYER))
        result.initialPlayer = player(requiredTable(table, server_constants::INITIAL_PLAYER));
    result.characterName = string(table, server_constants::CHARACTERNAME);
    result.hairModel = static_cast<uint16_t>(integer(table, server_constants::HAIRMODEL, 0, UINT16_MAX));
    result.shirtModel = static_cast<uint16_t>(integer(table, server_constants::SHIRTMODEL, 0, UINT16_MAX));
    result.pantsModel = static_cast<uint16_t>(integer(table, server_constants::PANTSMODEL, 0, UINT16_MAX));
    result.hairColor = color(requiredTable(table, server_constants::HAIRCOLOR));
    result.pantsColor = color(requiredTable(table, server_constants::PANTSCOLOR));
    return result;
}

GameIndexDTO PersistenceDeserializer::deserializeIndex(const toml::table& table) {
    GameIndexDTO result;
    result.id = string(table, server_constants::ID);
    result.farmName = string(table, server_constants::FARMNAME);
    result.mapPath = table[server_constants::MAPPATH].value<std::string>().value_or(server_constants::EMPTY);
    result.users = strings(requiredArray(table, server_constants::USERS));
    result.isMultiplayer = boolean(table, server_constants::ISMULTIPLAYER);
    const toml::table& previews = requiredTable(table, server_constants::PLAYERPREVIEWS);
    for (const auto& [userName, node] : previews) {
        if (!node.is_table()) {
            throw std::runtime_error(server_constants::INVALID_PLAYER_PREVIEW_IN_PERSISTENCE_INDEX);
        }
        result.playerPreviews.emplace(std::string(userName.str()), indexPlayer(*node.as_table()));
    }
    for (const std::string& userName : result.users) {
        if (result.playerPreviews.find(userName) == result.playerPreviews.end()) {
            throw std::runtime_error(server_constants::MISSING_PLAYER_PREVIEW_IN_INDEX_FOR_PREFIX + userName);
        }
    }
    return result;
}

PersistenceDTO PersistenceDeserializer::deserializeGame(const toml::table& table) {
    PersistenceDTO result;
    result.id = string(table, server_constants::ID);
    result.farmName = string(table, server_constants::FARMNAME);
    result.mapId = static_cast<uint8_t>(integer(table, server_constants::MAPID, 0, UINT8_MAX));
    if (table.contains(server_constants::WORLD_SEED))
        result.worldSeed = static_cast<uint32_t>(integer(table, server_constants::WORLD_SEED, 0, UINT32_MAX));
    result.currentDay = static_cast<uint32_t>(integer(table, server_constants::CURRENTDAY, 0, UINT32_MAX));
    result.users = strings(requiredArray(table, server_constants::USERS));
    const toml::table& players = requiredTable(table, server_constants::PLAYERS);
    for (const auto& [username, data] : players) {
        if (!data.is_table()) throw std::runtime_error(server_constants::INVALID_PLAYER_ENTRY);
        result.players.emplace(std::string(username.str()), player(*data.as_table()));
    }
    result.farm = farm(requiredTable(table, server_constants::FARM));
    if (table.contains(server_constants::INTERIORS)) {
        std::set<std::pair<InteriorId, uint8_t>> rooms;
        for (const auto& node: requiredArray(table, server_constants::INTERIORS)) {
            if (!node.is_table())
                throw std::runtime_error(server_constants::INVALID_SAVED_INTERIOR);
            auto room = interior(*node.as_table());
            if (room.interior_id != InteriorId::PlayerHouse || room.house_index >= 4 || !rooms.emplace(room.interior_id, room.house_index).second)
                throw std::runtime_error(server_constants::INVALID_OR_DUPLICATE_SAVED_HOUSE);
            result.interiors.push_back(std::move(room));
        }
    }
    if (table.contains(server_constants::DUNGEONFURNITURE)) {
        for (const auto& [key, node]: requiredTable(table, server_constants::DUNGEONFURNITURE)) {
            const auto level = numericKey(std::string(key.str()));
            if (level == 0 || level > 10 || !node.is_array() || result.dungeonFurniture.contains(static_cast<uint8_t>(level)))
                throw std::runtime_error(server_constants::INVALID_OR_DUPLICATE_SAVED_DUNGEON_LEVEL);
            auto& items = result.dungeonFurniture[static_cast<uint8_t>(level)];
            for (const auto& entry: *node.as_array()) {
                if (!entry.is_table())
                    throw std::runtime_error(server_constants::INVALID_DUNGEON_FURNITURE);
                auto item = furniture(*entry.as_table());
                if (item.type != FurnitureId::Chest)
                    throw std::runtime_error(server_constants::UNSUPPORTED_DUNGEON_FURNITURE);
                items.push_back(item);
            }
        }
    }

    if (table.contains(server_constants::DUNGEONSTORAGELOOT)) {
        for (const auto& [key, node]: requiredTable(table, server_constants::DUNGEONSTORAGELOOT)) {
            const auto level = numericKey(std::string(key.str()));
            if (level == 0 || level > 10 || !node.is_array() || !result.dungeonFurniture.contains(static_cast<uint8_t>(level)) ||
                result.dungeonStorageLoot.contains(static_cast<uint8_t>(level)))
                throw std::runtime_error(server_constants::INVALID_SAVED_DUNGEON_STORAGE_LOOT);
            auto& items = result.dungeonStorageLoot[static_cast<uint8_t>(level)];
            for (const auto& entry: *node.as_array()) {
                if (!entry.is_table())
                    throw std::runtime_error(server_constants::INVALID_DUNGEON_LOOT);
                items.push_back(loot(*entry.as_table()));
            }
        }
    }
    if (table.contains(server_constants::CHESTINVENTORIES)) {
        for (const auto& [key, node]: requiredTable(table, server_constants::CHESTINVENTORIES)) {
            const auto id = numericKey(std::string(key.str()));
            const auto* items = node.as_table();
            if (id == 0 || !items)
                throw std::runtime_error(server_constants::INVALID_CHEST_INVENTORY);
            std::map<uint8_t, Stack> contents;
            for (const auto& [slotKey, item]: *items) {
                const auto slot = numericKey(std::string(slotKey.str()));
                if (slot >= CHEST_INVENTORY_CAPACITY || !item.is_table() || !contents.emplace(static_cast<uint8_t>(slot), stack(*item.as_table())).second) {
                    throw std::runtime_error(server_constants::INVALID_CHEST_INVENTORY_SLOT);
                }
            }
            Inventory{CHEST_INVENTORY_CAPACITY}.restoreSlots(contents);
            if (!result.chestInventories.emplace(id, std::move(contents)).second) {
                throw std::runtime_error(server_constants::DUPLICATE_CHEST_INVENTORY);
            }
        }
    }
    std::set<uint32_t> chestIds;
    const auto validateChests = [&](const std::vector<FurnitureData>& items) {
        std::set<uint32_t> ids;
        for (const auto& item: items) {
            if (item.id == 0 || item.type > FurnitureId::Chest || item.dir > Direction::Right || !ids.insert(item.id).second)
                throw std::runtime_error(server_constants::INVALID_OR_DUPLICATE_FURNITURE);
            if (item.type == FurnitureId::Chest && (!chestIds.insert(item.id).second || !result.chestInventories.contains(item.id)))
                throw std::runtime_error(server_constants::DUPLICATE_CHEST_OR_MISSING_CONTENTS);
        }
    };
    validateChests(result.farm.furniture);
    for (const auto& room: result.interiors) validateChests(room.furniture);
    for (const auto& [level, items]: result.dungeonFurniture) validateChests(items);
    if (result.chestInventories.size() != chestIds.size())
        throw std::runtime_error(server_constants::MISSING_OR_ORPHANED_CHEST_INVENTORIES);
    if (table.contains(server_constants::EMPTYSOILELAPSED)) {
        for (const auto& entry : requiredArray(table, server_constants::EMPTYSOILELAPSED)) {
            const auto* soil = entry.as_table();
            if (!soil) throw std::runtime_error(server_constants::INVALID_EMPTY_SOIL_PROGRESS_ENTRY);
            result.emptySoilElapsed.emplace_back(position(requiredTable(*soil, server_constants::POS)),
                static_cast<uint64_t>(integer(*soil, server_constants::ELAPSED, 0,
                    std::numeric_limits<int64_t>::max())));
        }
    }
    result.cropGrowthProgress = uint64Map(requiredTable(table, server_constants::CROPGROWTHPROGRESS));
    result.cropWaterRemainingTime = uint64Map(requiredTable(table, server_constants::CROPWATERREMAININGTIME));
    result.harvestedCrops = boolMap(requiredTable(table, server_constants::HARVESTEDCROPS));
    if (table.contains(server_constants::REGROWINGCROPS)) {
        result.regrowingCrops = boolMap(requiredTable(table, server_constants::REGROWINGCROPS));
    }
    result.animalProductionProgress = uint64Map(requiredTable(table, server_constants::ANIMALPRODUCTIONPROGRESS));
    result.animalAvailableProducts = uint32Map(requiredTable(table, server_constants::ANIMALAVAILABLEPRODUCTS));
    result.communityCenterActionProgress = communityProgress(
        requiredTable(table, server_constants::COMMUNITYCENTERACTIONPROGRESS));
    return result;
}
