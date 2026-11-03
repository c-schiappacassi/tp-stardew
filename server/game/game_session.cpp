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
    if (dungeonWorldId && *dungeonWorldId != dto.worldId) {
        session.status.errorMsg ="La mazmorra cargada corresponde a otro mundo";
        return session;
    }

    name = dto.gameName;
    worldId = dto.worldId;

    PlayerData data{};
    data.hairModel = dto.hairModel;
    data.shirtModel = dto.shirtModel;
    data.pantsModel = dto.pantsModel;
    data.hairColor = dto.hairColor;
    data.pantsColor = dto.pantsColor;

    session = addPlayer(dto.playerName, data, output, "Partida creada");

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
    data.hairModel = dto.hairModel;
    data.shirtModel = dto.shirtModel;
    data.pantsModel = dto.pantsModel;
    data.hairColor = dto.hairColor;
    data.pantsColor = dto.pantsColor;

    return addPlayer(dto.playerName, data, output, "Te uniste a la partida");
}

GameSession Game::addPlayer(const std::string& playerName,const PlayerData& data,const std::shared_ptr<OutQueue>& output,const std::string& successMsg) {
    GameSession session{};
    session.playerName = playerName;
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