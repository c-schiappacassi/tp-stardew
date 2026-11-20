#include "game.h"

GameSession Game::createGame(const CreateGameDTO& dto,const std::shared_ptr<OutQueue>& output) {
    std::lock_guard<std::mutex> lock(mtx);
    GameSession session{};
    session.status.status = StatusCode::Error;

    if (configured) {
        session.status.errorMsg = "The game has already been created";
        return session;
    }
    if (dto.gameName.empty()) {
        session.status.errorMsg = "The game name is required";
        return session;
    }
    if (!world.acceptsWorld(dto.worldId)) {
        session.status.errorMsg ="The loaded dungeon belongs to another world";
        return session;
    }

    name = dto.gameName;
    worldId = dto.worldId;

    PlayerData data{};
    data.characterName = dto.characterName;
    data.hairModel = dto.hairModel;
    data.shirtModel = dto.shirtModel;
    data.pantsModel = dto.pantsModel;
    data.hairColor = dto.hairColor;
    data.pantsColor = dto.pantsColor;

    session = addPlayer(dto.userName, data, output, "Game created");

    if (session.status.status == StatusCode::Ok) {
        configured = true;
    }

    return session;
}

GameSession Game::joinGame(const JoinGameDTO& dto,const std::shared_ptr<OutQueue>& output) {
    std::lock_guard<std::mutex> lock(mtx);

    if (!configured || dto.code != id || community.finished()) {
        GameSession session{};
        session.status.status = StatusCode::Error;
        session.status.errorMsg = community.finished() ? "The game has already ended" :
            "No game exists with that code";
        return session;
    }

    PlayerData data{};
    data.characterName = dto.characterName;
    data.hairModel = dto.hairModel;
    data.shirtModel = dto.shirtModel;
    data.pantsModel = dto.pantsModel;
    data.hairColor = dto.hairColor;
    data.pantsColor = dto.pantsColor;

    return addPlayer(dto.userName, data, output, "You joined the game");
}

GameSession Game::addPlayer(const std::string& playerName,const PlayerData& data,const std::shared_ptr<OutQueue>& output,const std::string& successMsg) {
    GameSession session{};
    session.map = mapForSession();
    session.userName = playerName;
    session.status = playerManager.addPlayer(playerName, data, output);

    if (session.status.status != StatusCode::Ok) {
        return session;
    }

    session.status.gameId = id;
    session.status.worldId = worldId;
    session.status.successMsg = successMsg;
    session.status.gameCompleted = community.finished();
    session.gameId = id;
    session.playerId = session.status.playerId;
    session.worldId = worldId;
    session.input = input;
    ++saveRequests;

    return session;
}

GameSession Game::resumeGame(const ResumeGameDTO& dto, const std::shared_ptr<OutQueue>& output) {
    std::lock_guard<std::mutex> lock(mtx);
    GameSession session{};
    session.userName = dto.userName;
    session.status.status = StatusCode::Error;
    if (!configured || dto.gameId != id) {
        session.status.errorMsg = "That game does not exist";
        return session;
    }
    session.status = playerManager.resumePlayer(dto.userName, output);
    if (session.status.status != StatusCode::Ok) return session;
    session.status.gameId = id;
    session.status.worldId = worldId;
    session.status.successMsg = "Game resumed";
    session.status.gameCompleted = community.finished();
    session.gameId = id;
    session.playerId = session.status.playerId;
    session.worldId = worldId;
    session.input = input;
    session.map = mapForSession();
    ++saveRequests;
    return session;
}

std::optional<GameInfoDTO> Game::infoFor(const std::string& userName) const {
    std::lock_guard<std::mutex> lock(mtx);
    const auto data = playerManager.dataFor(userName);
    if (!data) return std::nullopt;
    return GameInfoDTO{id, name, *data, playerManager.isMultiplayer()};
}
