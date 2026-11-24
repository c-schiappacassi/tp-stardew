#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>

#include "chat/chat_services.h"
#include "commands/game_command_queue.h"
#include "commerce/commerce_manager.h"
#include "community/application/community_rewards.h"
#include "community/domain/community_center.h"
#include "config/game_config.h"
#include "game_types.h"
#include "persistence/persistence_dto.h"
#include "player/player_manager.h"
#include "sessions/game_sessions.h"
#include "thread.h"
#include "world/contracts/world_setup.h"
#include "world/world_manager.h"

class CommunityChat;

class Game: public Thread {
private:
    std::string id;
    std::string name;
    uint8_t worldId;
    uint32_t currentDay = 1;
    CommandQueue input;
    mutable std::mutex mtx;
    bool configured = false;
    bool endDayRequested = false;
    WorldManager world;
    std::shared_ptr<CommerceManager> commerce;
    CommunityCenter community;
    CommunityRewards communityRewards;
    std::shared_ptr<CommunityChat> communityChat;
    PlayerManager playerManager;
    GameSessions sessions;
    std::function<void(const PersistenceDTO&)> saveCallback;
    bool completionHandled = false;

    struct SaveRevision {
        uint64_t requests;
        uint64_t community;
        uint64_t rewards;
        bool operator==(const SaveRevision&) const = default;
    };
    uint64_t saveRequests = 0;
    std::optional<SaveRevision> savedRevision;

    GameSession completeEntry(GameSession session);
    PersistenceDTO buildPersistenceData() const;
    void saveIfNeeded(bool force = false);
    ServerStatus buildStatus() const;
    void processCommands();
    void sendStatus();
    void advancePlayers();
    void advanceWorld(std::chrono::milliseconds elapsed);
    void updateCommunity();
    void updateToolStates();
    void closeAll();

public:
    Game(const std::string& id, const std::string& name, uint8_t worldId, PreparedWorld prepared, const GameConfig& config = {}, ChatServices chatServices = {});

    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;
    Game(Game&&) = delete;
    Game& operator=(Game&&) = delete;

    PersistenceDTO getPersistenceData() const;
    void restorePersistenceData(const PersistenceDTO& saved);
    void setSaveCallback(std::function<void(const PersistenceDTO&)> callback);
    GameSession createGame(const CreateGameDTO& dto, const std::shared_ptr<OutQueue>& output);
    GameSession joinGame(const JoinGameDTO& dto, const std::shared_ptr<OutQueue>& output);
    GameSession resumeGame(const ResumeGameDTO& dto, const std::shared_ptr<OutQueue>& output);
    CommandSubmission enqueueCommand(uint16_t playerId, std::shared_ptr<CommandDTO> command);

    void run() override;
    void stop() override;
};
