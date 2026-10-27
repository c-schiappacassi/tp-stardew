#include "client_protocol.h"

#include <stdexcept>
#include <cstdint>


ClientProtocol::ClientProtocol(Socket& socket) : base(socket) {}


Position ClientProtocol::receivePosition() {
    int x = static_cast<int>(base.receiveUint16());
    int y = static_cast<int>(base.receiveUint16());
    return Position(x, y);
}

void ClientProtocol::send(const CreateGameDTO& dto) {
    base.sendUint8(static_cast<uint8_t>(MessageType::CreateGame));
    base.sendString(dto.playerName);
    base.sendString(dto.gameName);
    // TODO: faltan los datos de la apariencian
    base.sendUint8(dto.skin);
    base.sendUint8(dto.worldId);
}

void ClientProtocol::send(const JoinGameDTO& dto) {
    base.sendUint8(static_cast<uint8_t>(MessageType::JoinGame));
    base.sendString(dto.playerName);
    base.sendString(dto.code);
    base.sendUint8(dto.skin);
}

void ClientProtocol::send(const ResumeGameDTO& dto) {
    base.sendUint8(static_cast<uint8_t>(MessageType::ResumeGame));
    base.sendString(dto.playerName);
    base.sendUint16(dto.gameId);
}

void ClientProtocol::send(const MoveDTO& dto) {
    base.sendUint8(static_cast<uint8_t>(MessageType::Move));
    base.sendString(dto.playerName);
    base.sendUint16(static_cast<uint16_t>(dto.position.getX()));
    base.sendUint16(static_cast<uint16_t>(dto.position.getY()));
    base.sendUint8(static_cast<uint8_t>(dto.dir));
}

void ClientProtocol::send(const InteractDTO& dto) {
    base.sendUint8(static_cast<uint8_t>(MessageType::Interact));
    base.sendString(dto.playerName);
    base.sendUint8(static_cast<uint8_t>(dto.targetType));
    base.sendUint16(dto.targetId);
}

void ClientProtocol::send(const DonateDTO& dto) {
    base.sendUint8(static_cast<uint8_t>(MessageType::Donate));
    base.sendString(dto.playerName);
    base.sendUint8(dto.slot);
    base.sendUint8(dto.quantity);
}

void ClientProtocol::send(const ChatDTO& dto) {
    base.sendUint8(static_cast<uint8_t>(MessageType::Chat));
    base.sendString(dto.playerName);
    base.sendString(dto.text);
}

void ClientProtocol::send(const UseItemDTO& dto) {
    base.sendUint8(static_cast<uint8_t>(MessageType::UseItem));
    base.sendString(dto.playerName);
    base.sendUint8(dto.itemSlot);
}

void ClientProtocol::send(const RequestMultiplayerCodeDTO& dto) {
    base.sendUint8(static_cast<uint8_t>(MessageType::MultiplayerCode));
    base.sendString(dto.playerName);
}

void ClientProtocol::send(const DisconnectDTO& dto) {
    base.sendUint8(static_cast<uint8_t>(MessageType::ExitGame));
    base.sendString(dto.playerName);
}


void ClientProtocol::recv(ServerStatus& status) {
    const auto type = static_cast<MessageType>(base.receiveUint8());

    if (type != MessageType::Status) {
        throw std::runtime_error("Se esperaba un mensaje Status");
    }

    status.status = static_cast<StatusCode>(base.receiveUint8());
    status.errorMsg = base.receiveString();
    status.successMsg = base.receiveString();
    status.gameId = base.receiveString();
    status.playerId = base.receiveUint16();
    status.worldId = base.receiveUint8();
    //TODO: falta el map
}