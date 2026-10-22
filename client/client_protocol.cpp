#include "client_protocol.h"
#include <stdexcept>

#include <cstdint>

ClientProtocol::ClientProtocol(Socket& socket) : base(socket) {}

void ClientProtocol::send(const CreateGameDTO& dto) {
    base.sendUint8(static_cast<uint8_t>(MessageType::CreateGame));
    base.sendString(dto.gameName);
    base.sendString(dto.playerName);
    base.sendUint8(dto.skin);
    base.sendUint8(dto.worldId);
}

void ClientProtocol::send(const JoinGameDTO& dto) {
    base.sendUint8(static_cast<uint8_t>(MessageType::JoinGame));
    base.sendString(dto.code);
    base.sendString(dto.playerName);
    base.sendUint8(dto.skin);
}

/*
void ClientProtocol::send() {
    base.sendUint8(static_cast<uint8_t>(MessageType::ListGames));
}

void ClientProtocol::send(const ResumeGameDTO& dto) {
    base.sendUint8(static_cast<uint8_t>(MessageType::ResumeGame));
    base.sendUint16(dto.gameId);
}
*/

// TODO: terminar recv de server status cuando tengamos status definido
void ClientProtocol::recv(ServerStatus& status) {
    const auto type = static_cast<MessageType>(base.receiveUint8());

    if (type != MessageType::Status) {
        throw std::runtime_error("Se esperaba un mensaje Status");
    }

    status.status = static_cast<StatusCode>(base.receiveUint8());
    status.errorMsg = base.receiveString();
    status.successMsg = base.receiveString();
}

