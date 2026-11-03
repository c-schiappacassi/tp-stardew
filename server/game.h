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

        void disconnectPlayer(uint16_t playerId);
        void movePlayer(uint16_t playerId, const MoveDTO& move);
        void processCommands();
        void sendStatus();
        void advancePlayers();
        ServerStatus buildStatus() const;
        GameSession addPlayer(const std::string& userName,const PlayerData& data,const std::shared_ptr<OutQueue>& output,const std::string& successMsg);

    public:
        Game(const std::string& id, const std::string& name, uint8_t worldId, const char* servicename);

        const std::string& getId() const;
        std::string getName() const;
        uint8_t getWorldId() const;
        std::shared_ptr<CommandQueue> getInputQueue() const;

        GameSession createGame(const CreateGameDTO& dto,const std::shared_ptr<OutQueue>& output);
        GameSession joinGame(const JoinGameDTO& dto,const std::shared_ptr<OutQueue>& output);
        GameSession resumeGame(const ResumeGameDTO& dto, const std::shared_ptr<OutQueue>& output);

        void run() override;
        void stop() override;
};

#endif
