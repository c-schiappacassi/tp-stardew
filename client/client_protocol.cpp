#include "client_protocol.h"

#include <cstdint>
#include <stdexcept>

#include "protocol_errors.h"


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

Stack ClientProtocol::receiveStack() {
    Stack stack;
    stack.item_id = static_cast<ItemId>(base.receiveUint16());
    stack.quantity = base.receiveUint16();
    stack.qual = static_cast<Quality>(base.receiveUint8());
    return stack;
}

MineralData ClientProtocol::receiveMineralData() {
    MineralData mineral;
    mineral.id = base.receiveUint32();
    mineral.pos = receivePosition();
    mineral.type = static_cast<ResourceType>(base.receiveUint8());
    mineral.state = static_cast<MineralState>(base.receiveUint8());
    return mineral;
}

LevelData ClientProtocol::receiveLevelData() {
    LevelData level;
    level.entry = receivePosition();
    level.exit = receivePosition();

    const uint16_t count = base.receiveUint16();
    level.minerals.reserve(count);
    for (uint16_t i = 0; i < count; ++i) {
        level.minerals.push_back(receiveMineralData());
    }
    return level;
}

MineData ClientProtocol::receiveMineData() {
    MineData mine{};
    mine.current_level = base.receiveUint8();
    mine.current_level_data = receiveLevelData();
    return mine;
}


// TODO:
ZoneData ClientProtocol::receiveZoneData(Zone currentZone) {
    switch (currentZone) {
        case Zone::Dungeon : return ZoneData{receiveMineData()};
        case Zone::Farm : return ZoneData{receiveMineData()};
        case Zone::Town : return ZoneData{receiveMineData()};
    }

    throw ProtocolError("ZoneData desconocida");
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
    data.currentZone = static_cast<Zone>(base.receiveUint8());
    return data;
}

GameInfoDTO ClientProtocol::receiveGameInfo() {
    GameInfoDTO info;
    info.id = base.receiveString();
    info.farmName = base.receiveString();
    info.playerData = receivePlayerData();
    info.isMultiplayer = base.receiveUint8() != 0;
    return info;
}

PlayerDataLocal ClientProtocol::receivePlayerDataLocal() {
    PlayerData baseData = receivePlayerData();
    PlayerDataLocal localData(baseData);

    uint8_t itemCount = base.receiveUint8();
    localData.items.clear();
    for (uint8_t i = 0; i < itemCount; ++i) {
        uint8_t slot = base.receiveUint8();
        localData.items[slot] = receiveStack();
    }

    localData.current_slot = base.receiveUint8();
    localData.gold = base.receiveUint32();
    localData.health = base.receiveUint16();
    localData.maxHealth = base.receiveUint16();
    localData.energy = base.receiveUint16();
    localData.maxEnergy = base.receiveUint16();
    localData.zoneData = receiveZoneData(baseData.currentZone);

    return localData;
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
        case MessageType::SelectSlot: send(static_cast<const SelectSlotDTO&>(dto)); break;
        case MessageType::ListGames: send(static_cast<const ListGamesDTO&>(dto)); break;
        case MessageType::CheckCode: send(static_cast<const CheckCodeDTO&>(dto)); break;
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
    base.sendString(dto.gameId);
}

void ClientProtocol::send(const MoveDTO& dto) {
    base.sendUint8(static_cast<uint8_t>(MessageType::Move));
    base.sendString(dto.userName);
    base.sendUint8(static_cast<uint8_t>(dto.dir));
}

void ClientProtocol::send(const InteractDTO& dto) {
    base.sendUint8(static_cast<uint8_t>(MessageType::Interact));
    base.sendString(dto.userName);
    base.sendUint8(static_cast<uint8_t>(dto.dir));
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

void ClientProtocol::send(const SelectSlotDTO& dto) {
    base.sendUint8(static_cast<uint8_t>(MessageType::SelectSlot));
    base.sendString(dto.userName);
    base.sendUint8(dto.slot);
}

void ClientProtocol::send(const ListGamesDTO& dto) {
    base.sendUint8(static_cast<uint8_t>(MessageType::ListGames));
    base.sendString(dto.userName);
}

void ClientProtocol::send(const CheckCodeDTO& dto) {
    base.sendUint8(static_cast<uint8_t>(MessageType::CheckCode));
    base.sendString(dto.userName);
    base.sendString(dto.code);
}
// RECEIVERS

// ServerStatus { status, errorMsg, successMsg, gameId, playerId, worldId,
//                players, localPlayer }

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

    status.localPlayer = receivePlayerDataLocal();
}

// qtStatus { status, errorMsg, successMsg, games }

void ClientProtocol::recv(qtStatus& status) {
    status.status = static_cast<StatusCode>(base.receiveUint8());
    status.errorMsg = base.receiveString();
    status.successMsg = base.receiveString();

    const uint16_t gamesCount = base.receiveUint16();
    status.games.clear();
    status.games.reserve(gamesCount);
    for (uint16_t i = 0; i < gamesCount; ++i) {
        status.games.push_back(receiveGameInfo());
    }
}
