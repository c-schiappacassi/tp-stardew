#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "game/game_types.h"

class PlayerManager;
class WorldManager;

class GameSessions {
private:
    const std::string& gameId;
    const uint8_t& worldId;
    PlayerManager& players;
    const WorldManager& world;
    ConstMapData visuals;
    std::shared_ptr<const ConstMapData> map;

    std::shared_ptr<const ConstMapData> mapForSession();
    GameSession addPlayer(const std::string& name, const PlayerData& appearance, const std::shared_ptr<OutQueue>& output, const std::string& message);
    void complete(GameSession& session, const std::string& message) const;

public:
    GameSessions(const std::string& gameId, const uint8_t& worldId, PlayerManager& players, const WorldManager& world, ConstMapData visuals);

    GameSessions(const GameSessions&) = delete;
    GameSessions& operator=(const GameSessions&) = delete;
    GameSessions(GameSessions&&) = delete;
    GameSessions& operator=(GameSessions&&) = delete;

    static GameSession failure(const std::string& message);
    GameSession createPlayer(const CreateGameDTO& dto, const std::shared_ptr<OutQueue>& output);
    GameSession joinPlayer(const JoinGameDTO& dto, const std::shared_ptr<OutQueue>& output);
    GameSession resumePlayer(const std::string& name, const std::shared_ptr<OutQueue>& output);
};
