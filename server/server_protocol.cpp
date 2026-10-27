#include "server_protocol.h"

#include <cstdint>

ServerProtocol::ServerProtocol(Socket& socket) : base(socket) {}

MessageType ServerProtocol::receiveType() {
    return static_cast<MessageType>(base.receiveUint8());
}

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

// RECEIVERS (the type byte was already consumed by receiveType())

CreateGameDTO ServerProtocol::receiveCreateGame() {
    CreateGameDTO dto(base.receiveString());
    dto.gameName = base.receiveString();
    dto.worldId = base.receiveUint8();
    dto.hairModel = base.receiveUint16();
    dto.shirtModel = base.receiveUint16();
    dto.pantsModel = base.receiveUint16();
    dto.hairColor = receiveColor();
    dto.pantsColor = receiveColor();
    return dto;
}

JoinGameDTO ServerProtocol::receiveJoinGame() {
    JoinGameDTO dto(base.receiveString());
    dto.code = base.receiveString();
    dto.hairModel = base.receiveUint16();
    dto.shirtModel = base.receiveUint16();
    dto.pantsModel = base.receiveUint16();
    dto.hairColor = receiveColor();
    dto.pantsColor = receiveColor();
    return dto;
}

ResumeGameDTO ServerProtocol::receiveResumeGame() {
    ResumeGameDTO dto(base.receiveString());
    dto.gameId = base.receiveUint16();
    return dto;
}

MoveDTO ServerProtocol::receiveMove() {
    MoveDTO dto(base.receiveString());
    dto.position = receivePosition();
    dto.dir = static_cast<Direction>(base.receiveUint8());
    return dto;
}

InteractDTO ServerProtocol::receiveInteract() {
    InteractDTO dto(base.receiveString());
    dto.targetType = static_cast<TargetType>(base.receiveUint8());
    dto.targetId = base.receiveUint16();
    return dto;
}

DonateDTO ServerProtocol::receiveDonate() {
    DonateDTO dto(base.receiveString());
    dto.slot = base.receiveUint8();
    dto.quantity = base.receiveUint32();
    return dto;
}

ChatDTO ServerProtocol::receiveChat() {
    ChatDTO dto(base.receiveString());
    dto.text = base.receiveString();
    return dto;
}

UseItemDTO ServerProtocol::receiveUseItem() {
    UseItemDTO dto(base.receiveString());
    dto.itemSlot = base.receiveUint8();
    return dto;
}
/*
RequestMultiplayerCodeDTO ServerProtocol::receiveRequestMultiplayerCode() {
    return RequestMultiplayerCodeDTO(base.receiveString());
}

DisconnectDTO ServerProtocol::receiveDisconnect() {
    return DisconnectDTO(base.receiveString());
}
*/
// SENDERS

void ServerProtocol::sendStatus(const ServerStatus& status) {
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