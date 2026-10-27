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

// RECEIVERS

void ServerProtocol::receive(CreateGameDTO& dto) {
    dto.playerName = base.receiveString();
    dto.gameName = base.receiveString();
    dto.worldId = base.receiveUint8();

    // TODO: datos de apariencia
}

void ServerProtocol::receive(JoinGameDTO& dto) {
    dto.playerName = base.receiveString();
    dto.code = base.receiveString();
    dto.skin = base.receiveUint8();
}

void ServerProtocol::receive(ResumeGameDTO& dto) {
    dto.playerName = base.receiveString();
    dto.gameId = base.receiveUint16();
}

void ServerProtocol::receive(MoveDTO& dto) {
    dto.playerName = base.receiveString();
    dto.position = receivePosition();
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
    // TODO: map de players data
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
