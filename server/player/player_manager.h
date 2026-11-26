#pragma once

#include <chrono>
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "chat/chat_services.h"
#include "commands/command_router.h"
#include "player/chat/player_chat.h"
#include "player/time/day_cycle.h"
#include "ports/player_spawn_source.h"
#include "sessions/player_roster.h"
#include "status/player_status.h"
#include "updates/player_updates.h"

class PlayerManager {
private:
    PlayerRoster roster;
    DayCycle day;
    PlayerChat chat;
    CommandRouter commands;
    PlayerUpdates updates;
    PlayerStatus playerStatus;
    const PlayerSpawnSource& spawns;

    ServerStatus completeEntry(const PlayerRoster::Admission& admission);

public:
    PlayerManager(const PlayerConfig& config, const MovementRules& movement,
                  const ZoneDataSource& zones, PlayerInteractions& interactions,
                  const PlayerSpawnSource& spawns, uint8_t maxPlayers = 4,
                  ChatServices chatServices = {});

    PlayerManager(const PlayerManager&) = delete;
    PlayerManager& operator=(const PlayerManager&) = delete;
    PlayerManager(PlayerManager&&) = delete;
    PlayerManager& operator=(PlayerManager&&) = delete;

    ServerStatus addPlayer(const std::string& name, const PlayerData& appearance,const std::shared_ptr<OutQueue>& output, const PlayerRegistration& registerPlayer = {});
    ServerStatus resumePlayer(const std::string& name, const std::shared_ptr<OutQueue>& output);
    std::optional<PlayerData> dataFor(const std::string& name) const;
    std::map<std::string, PlayerDataLocal> getPersistencePlayers() const;
    std::vector<std::string> playerNames() const;
    void restorePlayers(const std::map<std::string, PlayerDataLocal>& saved, const std::vector<std::string>& names = {});
    bool isMultiplayer() const;
    std::vector<CollisionBounds> getCollisionBounds(Zone zone) const;
    std::vector<uint16_t> connectedPlayerIds() const;
    std::vector<Combatant> combatants() const;
    PlayerSnapshot snapshot(uint16_t playerId) const;
    std::map<uint8_t, Stack> inventory(uint16_t playerId) const;
    PlayerActionResult apply(uint16_t playerId, const PlayerAction& action);
    bool advanceDay(std::chrono::milliseconds elapsed);
    bool cancelSleep(uint16_t playerId);
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
    bool hasAvailableSlot() const;
};
