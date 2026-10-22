#ifndef GAME_H
#define GAME_H

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "game_types.h"
#include "player_record.h"
#include "thread.h"

class Game:public Thread {

    private:
        std::string id;
        std::string name;
        uint8_t worldId;
        std::shared_ptr<CommandQueue> input;
        std::mutex mtx;
        std::vector<PlayerRecord> players;
        uint16_t nextPlayerId = 1;

        void disconnectPlayer(uint16_t playerId);

    public:
        Game(const std::string& id, const std::string& name, uint8_t worldId);

        const std::string& getId() const;
        const std::string& getName() const;
        uint8_t getWorldId() const;
        std::shared_ptr<CommandQueue> getInputQueue() const;
        bool tryAddPlayer(const std::string& playerName,uint8_t skin,const std::shared_ptr<OutQueue>& output,uint16_t& assignedId);

        void run() override;
        void stop() override;
};

#endif