#include "server_protocol.h"

#include <cstdint>
#include <limits>
#include <utility>

#include "protocol_errors.h"

namespace {
constexpr size_t MAX_U8 = std::numeric_limits<uint8_t>::max();
constexpr size_t MAX_U16 = std::numeric_limits<uint16_t>::max();

// Los contadores viajan en 1 o 2 bytes: si no entran, se rechaza el mensaje
// ANTES de armarlo (si no, el contador se truncaba y el cliente se desincronizaba).
void validateCount(size_t count, size_t limit, const char* what) {
    if (count > limit) {
        throw ProtocolError(std::string("Demasiados elementos para el protocolo: ") + what);
    }
}

void validateFarmCounts(const FarmData& farm) {
    validateCount(farm.crops.size(), MAX_U16, "cultivos");
    validateCount(farm.animals.size(), MAX_U16, "animales");
    validateCount(farm.tilledSoil.size(), MAX_U16, "tierra arada");
    validateCount(farm.decorations.size(), MAX_U16, "decoraciones");
}

bool hasMatchingZoneData(const PlayerDataLocal& data) {
    switch (data.currentZone) {
        case Zone::Farm:
            return std::holds_alternative<FarmData>(data.zoneData);
        case Zone::Dungeon:
            return std::holds_alternative<MineData>(data.zoneData);
        case Zone::Town:
            return std::holds_alternative<std::monostate>(data.zoneData);
    }
    return false;
}

void validateStatus(const ServerStatus& status) {
    const auto& local = status.localPlayer;
    const bool emptyEntryError =
            status.status == StatusCode::Error &&
            std::holds_alternative<std::monostate>(local.zoneData) &&
            (local.currentZone == Zone::Farm || local.currentZone == Zone::Dungeon);
    if (!hasMatchingZoneData(local) && !emptyEntryError) {
        throw ProtocolError("Los datos locales no corresponden a la zona del jugador");
    }

    validateCount(status.players.size(), MAX_U16, "jugadores");
    validateCount(local.items.size(), MAX_U8, "items");
    if (const auto* farm = std::get_if<FarmData>(&local.zoneData)) {
        validateFarmCounts(*farm);
    }
    if (const auto* mine = std::get_if<MineData>(&local.zoneData)) {
        validateCount(mine->current_level_data.minerals.size(), MAX_U16, "minerales");
    }
}
}  // namespace

ServerProtocol::ServerProtocol(Socket&& socket) : base(std::move(socket)) {}

void ServerProtocol::shutdown() { base.shutdown(); }

MessageType ServerProtocol::receiveType() {
    return static_cast<MessageType>(base.receiveUint8());
}

Position ServerProtocol::receivePosition() {
    int x = static_cast<int>(base.receiveUint16());
    int y = static_cast<int>(base.receiveUint16());
    return Position(x, y);
}

void ServerProtocol::writePosition(MessageBuffer& out, const Position& pos) {
    out.putUint16(static_cast<uint16_t>(pos.getX()));
    out.putUint16(static_cast<uint16_t>(pos.getY()));
}

Color ServerProtocol::receiveColor() {
    Color color;
    color.r = base.receiveUint8();
    color.g = base.receiveUint8();
    color.b = base.receiveUint8();
    return color;
}

void ServerProtocol::writeColor(MessageBuffer& out, const Color& color) {
    out.putUint8(color.r);
    out.putUint8(color.g);
    out.putUint8(color.b);
}

Direction ServerProtocol::receiveDirection() {
    const uint8_t raw = base.receiveUint8();
    if (raw > static_cast<uint8_t>(Direction::Right)) {
        throw ProtocolError("direccion invalida"); // dir fuera de rango corta conexion?
    }
    return static_cast<Direction>(raw);
}

void ServerProtocol::writeStack(MessageBuffer& out, const Stack& stack) {
    out.putUint16(static_cast<uint16_t>(stack.item_id));
    out.putUint16(stack.quantity);
    out.putUint8(static_cast<uint8_t>(stack.qual));
}

void ServerProtocol::writePlayerData(MessageBuffer& out, const PlayerData& data) {
    out.putString(data.characterName);
    writePosition(out, data.pos);
    out.putUint8(static_cast<uint8_t>(data.state));
    out.putUint8(static_cast<uint8_t>(data.dir));
    out.putUint16(data.hairModel);
    out.putUint16(data.shirtModel);
    out.putUint16(data.pantsModel);
    writeColor(out, data.hairColor);
    writeColor(out, data.pantsColor);
    out.putUint8(static_cast<uint8_t>(data.currentZone));
    out.putUint16(data.heldItem ? static_cast<uint16_t>(*data.heldItem) : 0);
}

void ServerProtocol::writeGameInfo(MessageBuffer& out, const GameInfoDTO& info) {
    out.putString(info.id);
    out.putString(info.farmName);
    writePlayerData(out, info.playerData);
    out.putUint8(info.isMultiplayer ? 1 : 0);
}

void ServerProtocol::writeMineralData(MessageBuffer& out, const MineralData& mineral) {
    out.putUint32(mineral.id);
    writePosition(out, mineral.pos);
    out.putUint8(static_cast<uint8_t>(mineral.type));
    out.putUint8(static_cast<uint8_t>(mineral.state));
}

void ServerProtocol::writeLevelData(MessageBuffer& out, const LevelData& level) {
    writePosition(out, level.entry);
    writePosition(out, level.exit);

    out.putUint16(static_cast<uint16_t>(level.minerals.size()));
    for (const MineralData& mineral : level.minerals) {
        writeMineralData(out, mineral);
    }
}

void ServerProtocol::writeMineData(MessageBuffer& out, const MineData& mine) {
    out.putUint8(mine.current_level);
    writeLevelData(out, mine.current_level_data);
}

void ServerProtocol::writeCropData(MessageBuffer& out, const CropData& crop) {
    out.putUint32(crop.id);
    writePosition(out, crop.pos);
    out.putUint8(static_cast<uint8_t>(crop.type));
    out.putUint8(static_cast<uint8_t>(crop.state));
    out.putUint8(crop.wattered ? 1 : 0);
}

void ServerProtocol::writeAnimalData(MessageBuffer& out, const AnimalData& animal) {
    out.putUint32(animal.id);
    writePosition(out, animal.pos);
    out.putUint8(static_cast<uint8_t>(animal.dir));
    out.putUint8(static_cast<uint8_t>(animal.type));
    out.putUint8(static_cast<uint8_t>(animal.state));
    out.putUint8(animal.needs_interaction ? 1 : 0);
}

void ServerProtocol::writeDecorationData(MessageBuffer& out, const DecorationData& decoration) {
    out.putUint32(decoration.id);
    writePosition(out, decoration.pos);
    out.putUint16(static_cast<uint16_t>(decoration.item));
}

void ServerProtocol::writeFarmData(MessageBuffer& out, const FarmData& farm) {
    out.putUint16(static_cast<uint16_t>(farm.crops.size()));
    for (const auto& crop : farm.crops) {
        writeCropData(out, crop);
    }
    out.putUint16(static_cast<uint16_t>(farm.animals.size()));
    for (const auto& animal : farm.animals) {
        writeAnimalData(out, animal);
    }
    out.putUint16(static_cast<uint16_t>(farm.tilledSoil.size()));
    for (const auto& position : farm.tilledSoil) {
        writePosition(out, position);
    }
    out.putUint16(static_cast<uint16_t>(farm.decorations.size()));
    for (const auto& decoration : farm.decorations) {
        writeDecorationData(out, decoration);
    }
}

void ServerProtocol::writeZoneData(MessageBuffer& out, Zone currentZone,
                                   const ZoneData& zoneData) {
    switch (currentZone) {
        case Zone::Farm: {
            const auto* farm = std::get_if<FarmData>(&zoneData);
            writeFarmData(out, farm ? *farm : FarmData{});
            return;
        }
        case Zone::Dungeon: {
            const auto* mine = std::get_if<MineData>(&zoneData);
            writeMineData(out, mine ? *mine : MineData{});
            return;
        }
        case Zone::Town:
            return;
    }
}

void ServerProtocol::writePlayerDataLocal(MessageBuffer& out, const PlayerDataLocal& localData) {
    writePlayerData(out, localData);

    out.putUint8(static_cast<uint8_t>(localData.items.size()));
    for (const auto& [slot, stack] : localData.items) {
        out.putUint8(slot);
        writeStack(out, stack);
    }

    out.putUint8(localData.current_slot);
    out.putUint32(localData.gold);
    out.putUint16(localData.health);
    out.putUint16(localData.maxHealth);
    out.putUint16(localData.energy);
    out.putUint16(localData.maxEnergy);
    writeZoneData(out, localData.currentZone, localData.zoneData);
}

// RECEIVERS (the type byte --> consumed by receiveType())

SelectSlotDTO ServerProtocol::receiveSelectSlot() {
    SelectSlotDTO dto(base.receiveString());
    dto.slot = base.receiveUint8();
    return dto;
}

CreateGameDTO ServerProtocol::receiveCreateGame() {
    CreateGameDTO dto(base.receiveString());
    dto.characterName = base.receiveString();
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
    dto.characterName = base.receiveString();
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
    dto.gameId = base.receiveString();
    return dto;
}

MoveDTO ServerProtocol::receiveMove() {
    MoveDTO dto(base.receiveString());
    dto.dir = receiveDirection();
    return dto;
}

InteractDTO ServerProtocol::receiveInteract() {
    InteractDTO dto(base.receiveString());
    dto.dir = receiveDirection();
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

RequestMultiplayerCodeDTO ServerProtocol::receiveRequestMultiplayerCode() {
    return RequestMultiplayerCodeDTO(base.receiveString());
}

DisconnectDTO ServerProtocol::receiveDisconnect() {
    return DisconnectDTO(base.receiveString());
}

ListGamesDTO ServerProtocol::receiveListGames() {
    return ListGamesDTO(base.receiveString());
}

CheckCodeDTO ServerProtocol::receiveCheckCode() {
    CheckCodeDTO dto(base.receiveString());
    dto.code = base.receiveString();
    return dto;
}

// SENDERS (cada uno arma el mensaje entero y hace UN solo send)

void ServerProtocol::sendStatus(const ServerStatus& status) {
    validateStatus(status);

    MessageBuffer out(4096);
    out.putUint8(static_cast<uint8_t>(status.status));
    out.putString(status.errorMsg);
    out.putString(status.successMsg);
    out.putString(status.gameId);
    out.putUint16(status.playerId);
    out.putUint8(status.worldId);

    out.putUint16(static_cast<uint16_t>(status.players.size()));
    for (const auto& [name, data] : status.players) {
        out.putString(name);
        writePlayerData(out, data);
    }

    writePlayerDataLocal(out, status.localPlayer);
    base.send(out);
}

void ServerProtocol::sendQtStatus(const qtStatus& status) {
    validateCount(status.games.size(), MAX_U16, "partidas");

    MessageBuffer out(1024);
    out.putUint8(static_cast<uint8_t>(status.status));
    out.putString(status.errorMsg);
    out.putString(status.successMsg);

    out.putUint16(static_cast<uint16_t>(status.games.size()));
    for (const GameInfoDTO& info : status.games) {
        writeGameInfo(out, info);
    }
    base.send(out);
}
