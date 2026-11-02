#include "client_protocol.h"

#include <stdexcept>
#include <cstdint>


ClientProtocol::ClientProtocol(Socket& socket) : base(socket) {}


Position ClientProtocol::receivePosition() {
    int x = static_cast<int>(base.receiveUint16());
    int y = static_cast<int>(base.receiveUint16());
    return Position(x, y);
}

void ClientProtocol::sendPosition(const Position& pos) {
    base.sendUint16(static_cast<uint16_t>(pos.getX()));
    base.sendUint16(static_cast<uint16_t>(pos.getY()));
}

void ClientProtocol::sendColor(const Color& color) {
    base.sendUint8(color.r);
    base.sendUint8(color.g);
    base.sendUint8(color.b);
}

Color ClientProtocol::receiveColor() {
    Color color;
    color.r = base.receiveUint8();
    color.g = base.receiveUint8();
    color.b = base.receiveUint8();
    return color;
}

PlayerData ClientProtocol::receivePlayerData() {
    PlayerData data;
    data.characterName = base.receiveString();
    data.pos = receivePosition();
    data.state = static_cast<PlayerState>(base.receiveUint8());
    data.dir = static_cast<Direction>(base.receiveUint8());
    data.hairModel = base.receiveUint16();
    data.shirtModel = base.receiveUint16();
    data.pantsModel = base.receiveUint16();
    data.hairColor = receiveColor();
    data.pantsColor = receiveColor();
    return data;
}

// SENDERS

void ClientProtocol::send(const CommandDTO& dto) {
    switch (dto.type()) {
        case MessageType::CreateGame: send(static_cast<const CreateGameDTO&>(dto)); break;
        case MessageType::JoinGame: send(static_cast<const JoinGameDTO&>(dto)); break;
        case MessageType::ResumeGame: send(static_cast<const ResumeGameDTO&>(dto)); break;
        case MessageType::Move: send(static_cast<const MoveDTO&>(dto)); break;
        case MessageType::Interact: send(static_cast<const InteractDTO&>(dto)); break;
        case MessageType::Donate: send(static_cast<const DonateDTO&>(dto)); break;
        case MessageType::Chat: send(static_cast<const ChatDTO&>(dto)); break;
        case MessageType::UseItem: send(static_cast<const UseItemDTO&>(dto)); break;
        case MessageType::MultiplayerCode: send(static_cast<const RequestMultiplayerCodeDTO&>(dto)); break;
        case MessageType::ExitGame: send(static_cast<const DisconnectDTO&>(dto)); break;
        case MessageType::ListGames: send(static_cast<const RequestGameListDTO&>(dto)); break;
        default: throw std::invalid_argument("ClientProtocol: unsupported command");
    }
}

void ClientProtocol::send(const CreateGameDTO& dto) {
    base.sendUint8(static_cast<uint8_t>(MessageType::CreateGame));
    base.sendString(dto.userName);
    base.sendString(dto.characterName);
    base.sendString(dto.gameName);
    base.sendUint8(dto.worldId);
    base.sendUint16(dto.hairModel);
    base.sendUint16(dto.shirtModel);
    base.sendUint16(dto.pantsModel);
    sendColor(dto.hairColor);
    sendColor(dto.pantsColor);
}

void ClientProtocol::send(const JoinGameDTO& dto) {
    base.sendUint8(static_cast<uint8_t>(MessageType::JoinGame));
    base.sendString(dto.userName);
    base.sendString(dto.characterName);
    base.sendString(dto.code);
    base.sendUint16(dto.hairModel);
    base.sendUint16(dto.shirtModel);
    base.sendUint16(dto.pantsModel);
    sendColor(dto.hairColor);
    sendColor(dto.pantsColor);
}

void ClientProtocol::send(const ResumeGameDTO& dto) {
    base.sendUint8(static_cast<uint8_t>(MessageType::ResumeGame));
    base.sendString(dto.userName);
    base.sendUint16(dto.gameId);
}

void ClientProtocol::send(const MoveDTO& dto) {
    base.sendUint8(static_cast<uint8_t>(MessageType::Move));
    base.sendString(dto.userName);
    sendPosition(dto.position);
    base.sendUint8(static_cast<uint8_t>(dto.dir));
}

void ClientProtocol::send(const InteractDTO& dto) {
    base.sendUint8(static_cast<uint8_t>(MessageType::Interact));
    base.sendString(dto.userName);
    base.sendUint8(static_cast<uint8_t>(dto.targetType));
    base.sendUint16(dto.targetId);
}

void ClientProtocol::send(const DonateDTO& dto) {
    base.sendUint8(static_cast<uint8_t>(MessageType::Donate));
    base.sendString(dto.userName);
    base.sendUint8(dto.slot);
    base.sendUint32(dto.quantity);
}

void ClientProtocol::send(const ChatDTO& dto) {
    base.sendUint8(static_cast<uint8_t>(MessageType::Chat));
    base.sendString(dto.userName);
    base.sendString(dto.text);
}

void ClientProtocol::send(const UseItemDTO& dto) {
    base.sendUint8(static_cast<uint8_t>(MessageType::UseItem));
    base.sendString(dto.userName);
    base.sendUint8(dto.itemSlot);
}

void ClientProtocol::send(const RequestMultiplayerCodeDTO& dto) {
    base.sendUint8(static_cast<uint8_t>(MessageType::MultiplayerCode));
    base.sendString(dto.userName);
}

void ClientProtocol::send(const DisconnectDTO& dto) {
    base.sendUint8(static_cast<uint8_t>(MessageType::ExitGame));
    base.sendString(dto.userName);
}

void ClientProtocol::send(const RequestGameListDTO& dto) {
    base.sendUint8(static_cast<uint8_t>(MessageType::ListGames));
    base.sendString(dto.playerName);
}

// RECEIVERS

void ClientProtocol::recv(ServerStatus& status) {
    status.status = static_cast<StatusCode>(base.receiveUint8());
    status.errorMsg = base.receiveString();
    status.successMsg = base.receiveString();
    status.gameId = base.receiveString();
    status.playerId = base.receiveUint16();
    status.worldId = base.receiveUint8();

    uint16_t length = base.receiveUint16();
    status.players.clear();
    for (uint16_t i = 0; i < length; ++i) {
        std::string name = base.receiveString();
        status.players[name] = receivePlayerData();
    }
}

GameListDTO ClientProtocol::receiveGameList() {
    GameListDTO list;
    uint16_t count = base.receiveUint16();
    list.games.reserve(count);

    for (uint16_t i = 0; i < count; ++i) {
        GameInfoDTO game;
        game.id = base.receiveUint16();
        game.farmName = base.receiveString();
        game.playerData = receivePlayerData();
        game.isMultiplayer = (base.receiveUint8() != 0);
        list.games.push_back(std::move(game));
    }

    return list;
}
