#include "server_protocol.h"

#include <cstdint>


ServerProtocol::ServerProtocol(Socket& socket) : base(socket) {}

MessageType ServerProtocol::receiveType() {
    return static_cast<MessageType>(base.receiveUint8());
}

// VER POSITION.H PORQUE SE PUEDE HACER SIN INSTANCIAR UNA POSICION NUEVA TODO EL TIEMPO

Position ServerProtocol::receivePosition() {
    int x = static_cast<int>(base.receiveUint16());
    int y = static_cast<int>(base.receiveUint16());
    return Position(x, y);
}

void ServerProtocol::sendPosition(const Position& pos) {
    base.sendUint16(static_cast<uint16_t>(pos.getX()));
    base.sendUint16(static_cast<uint16_t>(pos.getY()));
}

Color ServerProtocol::receiveColor() {
    Color color;
    color.r = base.receiveUint8();
    color.g = base.receiveUint8();
    color.b = base.receiveUint8();
    return color;
}

void ServerProtocol::sendColor(const Color& color) {
    base.sendUint8(color.r);
    base.sendUint8(color.g);
    base.sendUint8(color.b);
}

void ServerProtocol::sendPlayerData(const PlayerData& data) {
    sendPosition(data.pos);
    base.sendUint8(static_cast<uint8_t>(data.state));
    base.sendUint8(static_cast<uint8_t>(data.dir));
    base.sendUint16(data.hairModel);
    base.sendUint16(data.shirtModel);
    base.sendUint16(data.pantsModel);
    sendColor(data.hairColor);
    sendColor(data.pantsColor);
}

// RECEIVERS

void ServerProtocol::receive(CreateGameDTO& dto) {
    dto.playerName = base.receiveString();
    dto.gameName = base.receiveString();
    dto.worldId = base.receiveUint8();
    dto.hairModel = base.receiveUint16();
    dto.shirtModel = base.receiveUint16();
    dto.pantsModel = base.receiveUint16();
    dto.hairColor = receiveColor();
    dto.pantsColor = receiveColor();
}

void ServerProtocol::receive(JoinGameDTO& dto) {
    dto.playerName = base.receiveString();
    dto.code = base.receiveString();
    dto.hairModel = base.receiveUint16();
    dto.shirtModel = base.receiveUint16();
    dto.pantsModel = base.receiveUint16();
    dto.hairColor = receiveColor();
    dto.pantsColor = receiveColor();
}

void ServerProtocol::receive(ResumeGameDTO& dto) {
    dto.playerName = base.receiveString();
    dto.gameId = base.receiveUint16();
}

void ServerProtocol::receive(MoveDTO& dto) {
    dto.playerName = base.receiveString();
    dto.position = receivePosition();
    dto.dir = static_cast<Direction>(base.receiveUint8());
}

void ServerProtocol::receive(InteractDTO& dto) {
    dto.playerName = base.receiveString();
    dto.targetType = static_cast<TargetType>(base.receiveUint8());
    dto.targetId = base.receiveUint16();
}

void ServerProtocol::receive(DonateDTO& dto) {
    dto.playerName = base.receiveString();
    dto.slot = base.receiveUint8();
    dto.quantity = base.receiveUint32();
}

void ServerProtocol::receive(ChatDTO& dto) {
    dto.playerName = base.receiveString();
    dto.text = base.receiveString();
}

void ServerProtocol::receive(UseItemDTO& dto) {
    dto.playerName = base.receiveString();
    dto.itemSlot = base.receiveUint8();
}

void ServerProtocol::receive(RequestMultiplayerCodeDTO& dto) {
    dto.playerName = base.receiveString();
}

void ServerProtocol::receive(DisconnectDTO& dto) {
    dto.playerName = base.receiveString();
}

// SENDERS

void ServerProtocol::send(const ServerStatus& status) {
    base.sendUint8(static_cast<uint8_t>(status.status));
    base.sendString(status.errorMsg);
    base.sendString(status.successMsg);
    base.sendString(status.gameId);
    base.sendUint16(status.playerId);
    base.sendUint8(status.worldId);

    base.sendUint16(static_cast<uint16_t>(status.players.size()));
    for (const auto& [name, data] : status.players) {
        base.sendString(name);
        sendPlayerData(data);
    }
}

// mismo que antes: para no romperles a ustedes, despues lo volamos

void ServerProtocol::send(const StatusDTO& dto) {
    base.sendUint8(static_cast<uint8_t>(MessageType::Status));
    base.sendUint8(static_cast<uint8_t>(dto.status));
    base.sendString(dto.errorMsg);
    base.sendString(dto.successMsg);
}

void ServerProtocol::send(const GameJoinedDTO& dto) {
    base.sendUint8(static_cast<uint8_t>(MessageType::GameJoined));
    base.sendString(dto.gameId);
    base.sendUint16(dto.playerId);
    base.sendUint8(dto.worldId);
}

void ServerProtocol::send(const GameListDTO& dto) {
    base.sendUint8(static_cast<uint8_t>(MessageType::GameList));
    base.sendUint16(static_cast<uint16_t>(dto.games.size()));
    for (const auto& game : dto.games) {
        base.sendUint16(game.id);
        base.sendString(game.gameName);
        base.sendString(game.playerName);
        base.sendUint8(game.skin);
        base.sendUint8(game.isMultiplayer ? 1 : 0);
    }
}
