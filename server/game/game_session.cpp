#include "game.h"

GameSession Game::createGame(const CreateGameDTO& dto,const std::shared_ptr<OutQueue>& output) {
    std::lock_guard<std::mutex> lock(mtx);
    GameSession session{};
    session.status.status = StatusCode::Error;

    if (configured) {
        session.status.errorMsg = "La partida ya fue creada";
        return session;
    }
    if (dto.gameName.empty()) {
        session.status.errorMsg = "El nombre de la partida es obligatorio";
        return session;
    }
    if (!world.acceptsWorld(dto.worldId)) {
        session.status.errorMsg ="La mazmorra cargada corresponde a otro mundo";
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

    session = addPlayer(dto.userName, data, output, "Partida creada");

    if (session.status.status == StatusCode::Ok) {
        configured = true;
    }

    return session;
}

GameSession Game::joinGame(const JoinGameDTO& dto,const std::shared_ptr<OutQueue>& output) {
    std::lock_guard<std::mutex> lock(mtx);

    if (!configured || dto.code != id) {
        GameSession session{};
        session.status.status = StatusCode::Error;
        session.status.errorMsg = "No existe una partida con ese codigo";
        return session;
    }

    PlayerData data{};
    data.characterName = dto.characterName;
    data.hairModel = dto.hairModel;
    data.shirtModel = dto.shirtModel;
    data.pantsModel = dto.pantsModel;
    data.hairColor = dto.hairColor;
    data.pantsColor = dto.pantsColor;

    return addPlayer(dto.userName, data, output, "Te uniste a la partida");
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
    session.gameId = id;
    session.playerId = session.status.playerId;
    session.worldId = worldId;
    session.input = input;

    return session;
}

GameSession Game::resumeGame(const ResumeGameDTO& dto, const std::shared_ptr<OutQueue>& output) {
    std::lock_guard<std::mutex> lock(mtx);
    GameSession session{};
    session.userName = dto.userName;
    session.status.status = StatusCode::Error;
    if (!configured || dto.gameId != id) {
        session.status.errorMsg = "No existe esa partida";
        return session;
    }
    session.status = playerManager.resumePlayer(dto.userName, output);
    if (session.status.status != StatusCode::Ok) return session;
    session.status.gameId = id;
    session.status.worldId = worldId;
    session.status.successMsg = "Partida retomada";
    session.gameId = id;
    session.playerId = session.status.playerId;
    session.worldId = worldId;
    session.input = input;
    session.map = mapForSession();
    return session;
}

std::optional<GameInfoDTO> Game::infoFor(const std::string& userName) const {
    std::lock_guard<std::mutex> lock(mtx);
    const auto data = playerManager.dataFor(userName);
    if (!data) return std::nullopt;
    return GameInfoDTO{id, name, *data, playerManager.isMultiplayer()};
}
