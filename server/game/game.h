#pragma once

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string>

#include "game_types.h"
#include "config/game_config.h"
#include "persistence/persistence_dto.h"
#include "chat/chat_services.h"
#include "thread.h"
#include "world/world_manager.h"
#include "player/player_manager.h"
#include "player/player_config.h"
#include "commerce/commerce_manager.h"
#include "community/domain/community_center.h"
#include "community/application/community_rewards.h"

class CommunityChat;

class Game : public Thread {

private:
    std::string id;
    std::string name;
    uint8_t worldId;
    uint32_t currentDay = 1;
    std::shared_ptr<CommandQueue> input;
    mutable std::mutex mtx;
    bool configured = false;
    bool endDayRequested = false;
    WorldManager world;
    std::shared_ptr<CommerceManager> commerce;
    CommunityCenter community;
    CommunityRewards communityRewards;
    std::shared_ptr<CommunityChat> communityChat;
    PlayerManager playerManager;
    std::shared_ptr<const ConstMapData> constMap;
    ConstMapData mapVisuals;
    bool hasMapVisuals = false;
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

    std::shared_ptr<const ConstMapData> mapForSession();
    PersistenceDTO buildPersistenceData() const;
    void saveIfNeeded(bool force = false);

    void processCommands();
    void sendStatus();
    void advancePlayers();
    bool advanceWorld(std::chrono::milliseconds elapsed);
    void updateCommunity();
    void updateToolStates();
    void closeAll();
    ServerStatus buildStatus() const;
    GameSession addPlayer(const std::string& playerName, const PlayerData& data,
                          const std::shared_ptr<OutQueue>& output, const std::string& successMsg);

public:
    Game(const std::string& id, const std::string& name, uint8_t worldId,
         const GameConfig& config = {}, uint8_t maxPlayers = 4, ChatServices chatServices = {});

    const std::string& getId() const;
    std::string getName() const;
    uint8_t getWorldId() const;
    PersistenceDTO getPersistenceData() const;
    void restorePersistenceData(const PersistenceDTO& saved);
    void setSaveCallback(std::function<void(const PersistenceDTO&)> callback);
    std::shared_ptr<CommandQueue> getInputQueue() const;

    GameSession createGame(const CreateGameDTO& dto,const std::shared_ptr<OutQueue>& output);
    GameSession joinGame(const JoinGameDTO& dto,const std::shared_ptr<OutQueue>& output);
    GameSession resumeGame(const ResumeGameDTO& dto, const std::shared_ptr<OutQueue>& output);
    std::optional<GameInfoDTO> infoFor(const std::string& userName) const;
    bool loadDungeon(uint8_t selectedWorldId,const std::map<uint8_t, DungeonRoomLayout>& layout);
    bool loadFarm(uint8_t selectedWorldId, const FarmLayout& layout);
    void setMapVisuals(ConstMapData visuals);
    bool loadTown(const TownConfig& config);
    bool configureDungeonEntrance(const DungeonEntrance& entrance);

    void run() override;
    void stop() override;
};
