#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "player/ports/combatant.h"
#include "player/ports/movement_rules.h"
#include "player/ports/player_spawn_source.h"

#include "player_factory.h"
#include "player_record.h"

class PlayerRoster {
public:
    struct Admission {
        PlayerRecord* record = nullptr;
        std::string error;
    };

    using iterator = std::map<uint16_t, PlayerRecord>::iterator;
    using const_iterator = std::map<uint16_t, PlayerRecord>::const_iterator;

private:
    std::map<uint16_t, PlayerRecord> players;
    uint16_t nextPlayerId = 1;
    uint8_t maxPlayers;
    PlayerFactory factory;
    const MovementRules& movement;
    const PlayerSpawnSource& spawns;

    PlayerRecord* findByName(const std::string& name);

public:
    PlayerRoster(const PlayerConfig& config, const MovementRules& movement,const PlayerSpawnSource& spawns, uint8_t maxPlayers);

    Admission addPlayer(const std::string& name, const PlayerData& appearance,const std::shared_ptr<OutQueue>& output);
    Admission resumePlayer(const std::string& name, const std::shared_ptr<OutQueue>& output);
    void restorePlayers(const std::map<std::string, PlayerDataLocal>& saved);
    PlayerRecord* find(uint16_t id);
    const PlayerRecord* find(uint16_t id) const;
    const PlayerRecord& require(uint16_t id) const;
    std::optional<PlayerData> dataFor(const std::string& name) const;
    bool isMultiplayer() const;
    bool hasConnectedPlayers() const;
    std::vector<CollisionBounds> getCollisionBounds(Zone zone) const;
    std::vector<uint16_t> connectedPlayerIds() const;
    std::vector<Combatant> combatants() const;

    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;
    std::size_t size() const;
};
