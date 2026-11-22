#pragma once

#include <chrono>
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>

#include "player/player_record.h"
#include "chat/chat_services.h"
#include "chat/player_chat.h"
#include "game/time/day_cycle.h"
#include "ports/combatant.h"
#include "ports/player_spawn_source.h"
#include "player_actions.h"
#include "player_factory.h"
#include "player_status.h"
#include "player_updates.h"

class PlayerManager {
private:
    std::map<uint16_t, PlayerRecord> players;
    uint16_t nextPlayerId = 1;
    uint8_t maxPlayers;
    PlayerFactory factory;
    PlayerUpdates updates;
    PlayerStatus playerStatus;
    PlayerActions actions;
    PlayerChat chat;
    const PlayerSpawnSource& spawns;
    const ZoneDataSource& zones;
    const MovementRules& movement;
    DayCycle day;

public:
    PlayerManager(const PlayerConfig& config, const MovementRules& movement, const ZoneDataSource& zones,
                  PlayerInteractions& interactions, const PlayerSpawnSource& spawns,
                  uint8_t maxPlayers = 4, ChatServices chatServices = {});

    ServerStatus addPlayer(const std::string& name,const PlayerData& appearance,const std::shared_ptr<OutQueue>& output);
    ServerStatus resumePlayer(const std::string& name, const std::shared_ptr<OutQueue>& output);
    std::optional<PlayerData> dataFor(const std::string& name) const;
    std::map<std::string, PlayerDataLocal> getPersistencePlayers() const;
    void restorePlayers(const std::map<std::string, PlayerDataLocal>& saved);
    bool isMultiplayer() const;
    std::vector<CollisionBounds> getCollisionBounds(Zone zone) const;
    std::vector<Combatant> getCombatants();
    bool advanceDay(std::chrono::milliseconds elapsed);
    bool isDayEnded() const;
    std::chrono::milliseconds remainingDayTime() const;
    bool hasConnectedPlayers() const;
    void collectCommunityRewards();
    void finishGame();
    void disconnectPlayer(uint16_t playerId);
    void closeOutputs();
    void updateToolStates(std::chrono::steady_clock::time_point now);
    void advancePlayers(int normalStep);
    void publishStatus(const ServerStatus& base);
    void handleCommand(const PlayerCommand& command, bool gameCompleted = false);
};
