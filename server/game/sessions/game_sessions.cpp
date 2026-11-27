#include "game_sessions.h"

#include "server_constants.h"

#include <stdexcept>
#include <utility>

#include "player/player_manager.h"
#include "world/world_manager.h"

template <typename Admission>
PlayerData GameSessions::appearanceOf(const Admission& dto) {
    PlayerData data{};
    data.characterName = dto.characterName;
    data.hairModel = dto.hairModel;
    data.shirtModel = dto.shirtModel;
    data.pantsModel = dto.pantsModel;
    data.hairColor = dto.hairColor;
    data.pantsColor = dto.pantsColor;
    return data;
}

void GameSessions::applyVisuals(BackgroundData& destination, BackgroundData& source) {
    destination.floor = std::move(source.floor);
    destination.decor = std::move(source.decor);
    destination.frame = std::move(source.frame);
    destination.buildings = std::move(source.buildings);
}

GameSessions::GameSessions(const std::string& gameId, const uint8_t& worldId, PlayerManager& players, const WorldManager& world, ConstMapData visuals):
    gameId(gameId), worldId(worldId), players(players), world(world), visuals(std::move(visuals)) {}

GameSession GameSessions::failure(const std::string& message) {
    GameSession session{};
    session.status.status = StatusCode::Error;
    session.status.errorMsg = message;
    return session;
}

std::shared_ptr<const ConstMapData> GameSessions::mapForSession() {
    if (!map) {
        auto assembled = world.buildConstMap();
        applyVisuals(assembled.farm, visuals.farm);
        applyVisuals(assembled.town, visuals.town);
        if (assembled.levels.size() != visuals.levels.size())
            throw std::runtime_error(server_constants::MINE_VISUALS_DO_NOT_MATCH_DUNGEON_LEVELS);
        for (std::size_t i = 0; i < assembled.levels.size(); ++i)
            applyVisuals(assembled.levels[i], visuals.levels[i]);
        map = std::make_shared<const ConstMapData>(std::move(assembled));
    }
    return map;
}

void GameSessions::complete(GameSession& session, const std::string& message) const {
    session.status.gameId = gameId;
    session.status.worldId = worldId;
    session.status.successMsg = message;
    session.gameId = gameId;
    session.playerId = session.status.playerId;
    session.worldId = worldId;
}

GameSession GameSessions::addPlayer(const std::string& name, const PlayerData& appearance, const std::shared_ptr<OutQueue>& output, const std::string& message, const PlayerRegistration& registerPlayer) {
    GameSession session{};
    session.map = mapForSession();
    session.userName = name;
    session.status = players.addPlayer(name, appearance, output, registerPlayer);
    if (session.status.status == StatusCode::Ok)
        complete(session, message);
    return session;
}

GameSession GameSessions::createPlayer(const CreateGameDTO& dto, const std::shared_ptr<OutQueue>& output) {
    return addPlayer(dto.userName, appearanceOf(dto), output, server_constants::GAME_CREATED);
}

GameSession GameSessions::joinPlayer(const JoinGameDTO& dto, const std::shared_ptr<OutQueue>& output, const PlayerRegistration& registerPlayer) {
    return addPlayer(dto.userName, appearanceOf(dto), output, server_constants::YOU_JOINED_THE_GAME, registerPlayer);
}

GameSession GameSessions::resumePlayer(const std::string& name, const std::shared_ptr<OutQueue>& output) {
    GameSession session{};
    session.userName = name;
    session.status = players.resumePlayer(name, output);
    if (session.status.status != StatusCode::Ok)
        return session;
    complete(session, server_constants::GAME_RESUMED);
    session.map = mapForSession();
    return session;
}
