#ifndef GAME_H
#define GAME_H

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <map>

#include "game_types.h"
#include "player_record.h"
#include "thread.h"
#include "network/acceptor.h"
#include "socket.h"
#include "entities/entity_id_generator.h"
#include "dungeon/dungeon.h"
#include "dungeon/dungeon_entrance.h"
#include "player/player_config.h"
#include <optional>

class Game:public Thread {

    private:
        std::string id;
        std::string name;
        uint8_t worldId;
        std::shared_ptr<CommandQueue> input;
        mutable std::mutex mtx;
        bool configured = false;
        std::map<uint16_t, PlayerRecord> players;
        uint16_t nextPlayerId = 1;
        Socket skt;
        Acceptor acceptor;
        Dungeon dungeon;
        EntityIdGenerator entityIds;
        std::optional<uint8_t> dungeonWorldId;
        PlayerConfig playerConfig;
        std::optional<DungeonEntrance> dungeonEntrance;

        void selectSlot(uint16_t playerId, uint8_t slot);
        void disconnectPlayer(uint16_t playerId);
        void movePlayer(uint16_t playerId, const MoveDTO& move);
        void processCommands();
        void sendStatus();
        void advancePlayers();
        ServerStatus buildStatus() const;
        void completePlayerStatus(ServerStatus& status,const PlayerRecord& record,bool includeBroken = false) const;
        GameSession addPlayer(const std::string& playerName,const PlayerData& data,const std::shared_ptr<OutQueue>& output,const std::string& successMsg);
        void updateToolStates();
        void selectPlayerSlot(uint16_t playerId,uint8_t slot);
        void usePlayerItem(uint16_t playerId,uint8_t slot);
        void interactPlayer(uint16_t playerId,const InteractDTO& interaction);
        bool useDungeonPassage(Player& player);

    public:
        Game(const std::string& id,const std::string& name,uint8_t worldId,const char* servicename,const PlayerConfig& playerConfig = {});

        const std::string& getId() const;
        std::string getName() const;
        uint8_t getWorldId() const;
        std::shared_ptr<CommandQueue> getInputQueue() const;
        GameSession createGame(const CreateGameDTO& dto,const std::shared_ptr<OutQueue>& output);
        GameSession joinGame(const JoinGameDTO& dto,const std::shared_ptr<OutQueue>& output);
        bool loadDungeon(uint8_t selectedWorldId,const std::map<uint8_t, DungeonRoomLayout>& layout);
        bool configureDungeonEntrance(const DungeonEntrance& entrance);

        void run() override;
        void stop() override;
};

#endif