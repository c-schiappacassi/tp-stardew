#include "player_roster.h"

#include "server_constants.h"

#include <algorithm>
#include <set>
#include <stdexcept>
#include <utility>

PlayerRoster::PlayerRoster(const PlayerConfig& config, const MovementRules& movement,const PlayerSpawnSource& spawns, uint8_t maxPlayers):
        maxPlayers(maxPlayers), factory(config), movement(movement), spawns(spawns) {}

PlayerRoster::Admission PlayerRoster::addPlayer(const std::string& name,const PlayerData& appearance,const std::shared_ptr<OutQueue>& output) {
    if (name.empty() || !output)
        return {nullptr, server_constants::COULD_NOT_REGISTER_THE_PLAYER};
    if (findByName(name))
        return {nullptr, server_constants::A_PLAYER_WITH_THAT_NAME_ALREADY_EXISTS};
    if (std::any_of(players.begin(), players.end(), [&](const auto& entry) {
            return entry.second.getPlayer().getCharacterName() == appearance.characterName;
        }))
        return {nullptr, server_constants::A_CHARACTER_WITH_THAT_NAME_ALREADY_EXISTS_IN_THIS_GAME};
    if (!hasAvailableSlot())
        return {nullptr, server_constants::THE_GAME_IS_FULL};
    auto player = factory.create(appearance);
    const auto spawn = spawns.spawnFor(player.snapshot());
    if (!spawn ||!player.restoreLocation(spawn->zone, spawn->level, spawn->position, spawn->interior))
        return {nullptr, server_constants::NO_POSITION_IS_AVAILABLE_FOR_THE_PLAYER};

    const auto it = players.emplace(nextPlayerId,PlayerRecord{nextPlayerId, name, std::move(player), output}).first;
    ++nextPlayerId;
    return {&it->second, {}};
}

PlayerRoster::Admission PlayerRoster::resumePlayer(const std::string& name,const std::shared_ptr<OutQueue>& output) {
    auto* record = findByName(name);
    if (!record)
        return {nullptr, server_constants::YOU_DO_NOT_HAVE_A_PLAYER_IN_THIS_GAME};
    if (record->isConnected() || !output)
        return {nullptr, server_constants::YOU_ARE_ALREADY_CONNECTED_TO_THIS_GAME};

    auto& player = record->getPlayer();
    if (!movement.canMove(player.snapshot(), player.getPosition())) {
        const auto spawn = spawns.spawnFor(player.snapshot());
        if (!spawn ||!player.restoreLocation(spawn->zone, spawn->level, spawn->position, spawn->interior))
            return {nullptr, server_constants::NO_POSITION_IS_AVAILABLE_TO_RESUME_THE_GAME};
    }

    record->connect(output);
    return {record, {}};
}

void PlayerRoster::restorePlayers(const std::map<std::string, PlayerDataLocal>& saved) {
    if (!players.empty() || saved.size() > maxPlayers || saved.size() > UINT16_MAX)
        throw std::runtime_error(server_constants::CANNOT_RESTORE_PLAYER_ROSTER);

    std::map<uint16_t, PlayerRecord> restored;
    std::set<std::string> characterNames;
    uint16_t id = 1;
    for (const auto& [username, data]: saved) {
        if (username.empty())
            throw std::runtime_error(server_constants::EMPTY_USERNAME_IN_SAVE);
        if (!characterNames.insert(data.characterName).second)
            throw std::runtime_error(server_constants::DUPLICATE_CHARACTER_NAME_IN_SAVE);
        restored.emplace(id, PlayerRecord{id, username, factory.restore(data)});
        ++id;
    }
    players = std::move(restored);
    nextPlayerId = id;
}

PlayerRecord* PlayerRoster::find(uint16_t id) {
    const auto it = players.find(id);
    return it == players.end() ? nullptr : &it->second;
}

PlayerRecord* PlayerRoster::findByName(const std::string& name) {
    for (auto& [id, record]: players) {
        if (record.getName() == name)
            return &record;
    }
    return nullptr;
}

std::optional<PlayerData> PlayerRoster::dataFor(const std::string& name) const {
    for (const auto& [id, record]: players) {
        if (record.getName() == name)
            return record.getPlayer().getPublicData();
    }
    return std::nullopt;
}

bool PlayerRoster::isMultiplayer() const { return players.size() > 1; }

bool PlayerRoster::hasConnectedPlayers() const {
    return std::any_of(players.begin(), players.end(),[](const auto& entry) { return entry.second.isConnected(); });
}

std::vector<CollisionBounds> PlayerRoster::getCollisionBounds(Zone zone) const {
    std::vector<CollisionBounds> result;
    for (const auto& [id, record]: players) {
        const auto& player = record.getPlayer();
        if (record.isConnected() && player.getZone() == zone)
            result.push_back(player.getCollisionBounds(player.getPosition()));
    }
    return result;
}

std::vector<uint16_t> PlayerRoster::connectedPlayerIds() const {
    std::vector<uint16_t> result;
    for (auto& [id, record]: players) {
        if (record.isConnected())
            result.push_back(id);
    }
    return result;
}

PlayerRoster::iterator PlayerRoster::begin() { return players.begin(); }

PlayerRoster::iterator PlayerRoster::end() { return players.end(); }

PlayerRoster::const_iterator PlayerRoster::begin() const { return players.begin(); }

PlayerRoster::const_iterator PlayerRoster::end() const { return players.end(); }

std::size_t PlayerRoster::size() const { return players.size(); }

const PlayerRecord* PlayerRoster::find(uint16_t id) const {
    const auto it = players.find(id);
    return it == players.end() ? nullptr : &it->second;
}

std::vector<Combatant> PlayerRoster::combatants() const {
    std::vector<Combatant> result;
    result.reserve(players.size());
    for (const auto& [id, record]: players) {
        if (record.isConnected())
            result.push_back({id, record.getPlayer().snapshot()});
    }
    return result;
}

bool PlayerRoster::hasAvailableSlot() const {
    return players.size() < maxPlayers;
}

const PlayerRecord& PlayerRoster::require(uint16_t id) const {
    const auto* record = find(id);
    if (!record)
        throw std::out_of_range(server_constants::UNKNOWN_PLAYER_ID);
    return *record;
}
