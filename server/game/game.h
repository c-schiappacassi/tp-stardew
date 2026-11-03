#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string>

#include "game_types.h"
#include "thread.h"
#include "world/world_manager.h"
#include "player/player_manager.h"
#include "player/player_config.h"

class Game:public Thread {

    private:
        std::string id;
        std::string name;
        uint8_t worldId;
        std::shared_ptr<CommandQueue> input;
        mutable std::mutex mtx;
        bool configured = false;
        WorldManager world;
        PlayerManager playerManager;

        void processCommands();
        void sendStatus();
        void advancePlayers();
        void updateToolStates();
        ServerStatus buildStatus() const;
        GameSession addPlayer(const std::string& playerName,const PlayerData& data,const std::shared_ptr<OutQueue>& output,const std::string& successMsg);
    public:
        Game(const std::string& id,const std::string& name,uint8_t worldId,const PlayerConfig& playerConfig = {},const DungeonConfig& dungeonConfig = {});

        const std::string& getId() const;
        std::string getName() const;
        uint8_t getWorldId() const;
        std::shared_ptr<CommandQueue> getInputQueue() const;

        GameSession createGame(const CreateGameDTO& dto,const std::shared_ptr<OutQueue>& output);
        GameSession joinGame(const JoinGameDTO& dto,const std::shared_ptr<OutQueue>& output);
        GameSession resumeGame(const ResumeGameDTO& dto, const std::shared_ptr<OutQueue>& output);
        std::optional<GameInfoDTO> infoFor(const std::string& userName) const;
        bool loadDungeon(uint8_t selectedWorldId,const std::map<uint8_t, DungeonRoomLayout>& layout);
        bool configureDungeonEntrance(const DungeonEntrance& entrance);

        void run() override;
        void stop() override;
};
