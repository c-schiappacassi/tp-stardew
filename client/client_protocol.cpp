#include "client_protocol.h"

#include <cstdint>
#include <stdexcept>
#include <utility>

#include "protocol_errors.h"


ClientProtocol::ClientProtocol(Socket&& socket) : base(std::move(socket)) {}


void ClientProtocol::shutdown() { base.shutdown(); }

Position ClientProtocol::receivePosition() {
    int x = static_cast<int>(base.receiveUint16());
    int y = static_cast<int>(base.receiveUint16());
    return Position(x, y);
}

void ClientProtocol::writePosition(MessageBuffer& out, const Position& pos) {
    out.putUint16(static_cast<uint16_t>(pos.getX()));
    out.putUint16(static_cast<uint16_t>(pos.getY()));
}

void ClientProtocol::writeColor(MessageBuffer& out, const Color& color) {
    out.putUint8(color.r);
    out.putUint8(color.g);
    out.putUint8(color.b);
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

EnemySnapshot ClientProtocol::receiveEnemyData() {
    EnemySnapshot enemy;
    enemy.id = base.receiveUint32();
    enemy.pos = receivePosition();
    enemy.dir = static_cast<Direction>(base.receiveUint8());
    enemy.type = static_cast<EnemyType>(base.receiveUint8());
    enemy.state = static_cast<EnemyState>(base.receiveUint8());
    enemy.health = base.receiveUint16();
    enemy.maxHealth = base.receiveUint16();
    enemy.frenzied = base.receiveUint8() != 0;
    enemy.action = base.receiveUint32();
    return enemy;
}

ProjectileData ClientProtocol::receiveProjectileData() {
    ProjectileData projectile;
    projectile.id = base.receiveUint32();
    projectile.pos = receivePosition();
    projectile.type = static_cast<FlameType>(base.receiveUint8());
    return projectile;
}

TrapData ClientProtocol::receiveTrapData() {
    TrapData trap;
    trap.id = base.receiveUint32();
    trap.pos = receivePosition();
    trap.dir = static_cast<Direction>(base.receiveUint8());
    trap.type = static_cast<TrapType>(base.receiveUint8());
    trap.state = static_cast<TrapState>(base.receiveUint8());
    trap.range = base.receiveUint16();
    return trap;
}

LootData ClientProtocol::receiveLootData() {
    LootData loot;
    loot.id = base.receiveUint32();
    loot.pos = receivePosition();
    loot.item = static_cast<ItemId>(base.receiveUint16());
    loot.quantity = base.receiveUint16();
    return loot;
}

DungeonCombatData ClientProtocol::receiveCombatData() {
    DungeonCombatData combat;
    combat.visitId = base.receiveUint32();
    combat.room = base.receiveUint8();
    auto count = base.receiveUint16();
    combat.enemies.reserve(count);
    for (uint16_t i = 0; i < count; ++i) {
        combat.enemies.push_back(receiveEnemyData());
    }
    count = base.receiveUint16();
    combat.projectiles.reserve(count);
    for (uint16_t i = 0; i < count; ++i) {
        combat.projectiles.push_back(receiveProjectileData());
    }
    count = base.receiveUint16();
    combat.traps.reserve(count);
    for (uint16_t i = 0; i < count; ++i) {
        combat.traps.push_back(receiveTrapData());
    }
    count = base.receiveUint16();
    combat.loot.reserve(count);
    for (uint16_t i = 0; i < count; ++i) {
        combat.loot.push_back(receiveLootData());
    }
    return combat;
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
    level.combat = receiveCombatData();
    return level;
}

MineData ClientProtocol::receiveMineData() {
    MineData mine{};
    mine.current_level = base.receiveUint8();
    mine.current_level_data = receiveLevelData();
    return mine;
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
    const uint16_t heldItem = base.receiveUint16();
    if (heldItem != 0) data.heldItem = static_cast<ItemId>(heldItem);
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
    localData.combatProgress.skeletons = base.receiveUint8() != 0;
    localData.combatProgress.vampires = base.receiveUint8() != 0;
    localData.combatProgress.orcs = base.receiveUint8() != 0;
    localData.combatProgress.skull = base.receiveUint8() != 0;

    return localData;
}

CropData ClientProtocol::receiveCropData() {
    CropData c;
    c.id = base.receiveUint32();
    c.pos = receivePosition();
    c.type = static_cast<CropType>(base.receiveUint8());
    c.state = static_cast<CropState>(base.receiveUint8());
    c.wattered = base.receiveUint8() != 0;
    return c;
}

AnimalData ClientProtocol::receiveAnimalData() {
    AnimalData a;
    a.id = base.receiveUint32();
    a.pos = receivePosition();
    a.dir = static_cast<Direction>(base.receiveUint8());
    a.type = static_cast<AnimalType>(base.receiveUint8());
    a.state = static_cast<AnimalState>(base.receiveUint8());
    a.needs_interaction = base.receiveUint8() != 0;
    return a;
}

FarmData ClientProtocol::receiveFarmData() {
    FarmData farm;
    uint16_t n = base.receiveUint16();
    for (uint16_t i = 0; i < n; ++i) farm.crops.push_back(receiveCropData());
    n = base.receiveUint16();
    for (uint16_t i = 0; i < n; ++i) farm.animals.push_back(receiveAnimalData());
    n = base.receiveUint16();
    for (uint16_t i = 0; i < n; ++i) farm.tilledSoil.push_back(receivePosition());
    return farm;
}

ZoneData ClientProtocol::receiveZoneData(Zone currentZone) {
    switch (currentZone) {
        case Zone::Dungeon: return ZoneData{receiveMineData()};
        case Zone::Farm:    return ZoneData{receiveFarmData()};
        case Zone::Town:    return ZoneData{std::monostate{}};
    }
    throw ProtocolError("ZoneData desconocida");
}

// SENDERS

void ClientProtocol::send(const CommandDTO& dto) {
    switch (dto.type()) {
        case MessageType::CreateGame: send(static_cast<const CreateGameDTO&>(dto)); break;
        case MessageType::JoinGame: send(static_cast<const JoinGameDTO&>(dto)); break;
        case MessageType::ResumeGame: send(static_cast<const ResumeGameDTO&>(dto)); break;
        case MessageType::Move: send(static_cast<const MoveDTO&>(dto)); break;
        case MessageType::Interact: send(static_cast<const InteractDTO&>(dto)); break;
        case MessageType::DayOk:send(static_cast<const DayOkDTO&>(dto));break;
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
    MessageBuffer out;
    out.putUint8(static_cast<uint8_t>(MessageType::CreateGame));
    out.putString(dto.userName);
    out.putString(dto.characterName);
    out.putString(dto.gameName);
    out.putUint8(dto.worldId);
    out.putUint16(dto.hairModel);
    out.putUint16(dto.shirtModel);
    out.putUint16(dto.pantsModel);
    writeColor(out, dto.hairColor);
    writeColor(out, dto.pantsColor);
    base.send(out);
}

void ClientProtocol::send(const JoinGameDTO& dto) {
    MessageBuffer out;
    out.putUint8(static_cast<uint8_t>(MessageType::JoinGame));
    out.putString(dto.userName);
    out.putString(dto.characterName);
    out.putString(dto.code);
    out.putUint16(dto.hairModel);
    out.putUint16(dto.shirtModel);
    out.putUint16(dto.pantsModel);
    writeColor(out, dto.hairColor);
    writeColor(out, dto.pantsColor);
    base.send(out);
}

void ClientProtocol::send(const ResumeGameDTO& dto) {
    MessageBuffer out;
    out.putUint8(static_cast<uint8_t>(MessageType::ResumeGame));
    out.putString(dto.userName);
    out.putString(dto.gameId);
    base.send(out);
}

void ClientProtocol::send(const MoveDTO& dto) {
    MessageBuffer out;
    out.putUint8(static_cast<uint8_t>(MessageType::Move));
    out.putString(dto.userName);
    out.putUint8(static_cast<uint8_t>(dto.dir));
    base.send(out);
}

void ClientProtocol::send(const InteractDTO& dto) {
    MessageBuffer out;
    out.putUint8(static_cast<uint8_t>(MessageType::Interact));
    out.putString(dto.userName);
    out.putUint8(static_cast<uint8_t>(dto.dir));
    base.send(out);
}

void ClientProtocol::send(const DonateDTO& dto) {
    MessageBuffer out;
    out.putUint8(static_cast<uint8_t>(MessageType::Donate));
    out.putString(dto.userName);
    out.putUint8(dto.slot);
    out.putUint32(dto.quantity);
    base.send(out);
}

void ClientProtocol::send(const DayOkDTO& dto) {
    MessageBuffer out;
    out.putUint8(static_cast<uint8_t>(MessageType::DayOk));
    out.putString(dto.userName);
    base.send(out);
}

void ClientProtocol::send(const ChatDTO& dto) {
    MessageBuffer out;
    out.putUint8(static_cast<uint8_t>(MessageType::Chat));
    out.putString(dto.userName);
    out.putString(dto.text);
    base.send(out);
}

void ClientProtocol::send(const UseItemDTO& dto) {
    MessageBuffer out;
    out.putUint8(static_cast<uint8_t>(MessageType::UseItem));
    out.putString(dto.userName);
    out.putUint8(dto.itemSlot);
    base.send(out);
}

void ClientProtocol::send(const RequestMultiplayerCodeDTO& dto) {
    MessageBuffer out;
    out.putUint8(static_cast<uint8_t>(MessageType::MultiplayerCode));
    out.putString(dto.userName);
    base.send(out);
}

void ClientProtocol::send(const DisconnectDTO& dto) {
    MessageBuffer out;
    out.putUint8(static_cast<uint8_t>(MessageType::ExitGame));
    out.putString(dto.userName);
    base.send(out);
}

void ClientProtocol::send(const SelectSlotDTO& dto) {
    MessageBuffer out;
    out.putUint8(static_cast<uint8_t>(MessageType::SelectSlot));
    out.putString(dto.userName);
    out.putUint8(dto.slot);
    base.send(out);
}

void ClientProtocol::send(const ListGamesDTO& dto) {
    MessageBuffer out;
    out.putUint8(static_cast<uint8_t>(MessageType::ListGames));
    out.putString(dto.userName);
    base.send(out);
}

void ClientProtocol::send(const CheckCodeDTO& dto) {
    MessageBuffer out;
    out.putUint8(static_cast<uint8_t>(MessageType::CheckCode));
    out.putString(dto.userName);
    out.putString(dto.code);
    base.send(out);
}

// RECEIVERS

// ServerStatus { status, errorMsg, successMsg, gameId, playerId, worldId,
//                players, localPlayer, dayEnded }

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
    status.dayEnded = base.receiveUint8() != 0;
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


BackgroundData ClientProtocol::receiveBackgroundData() {
    constexpr uint32_t MAX_FLOOR = 1024 * 1024;
    BackgroundData bg;

    const uint32_t nFloor = base.receiveUint32();
    if (nFloor > MAX_FLOOR) throw ProtocolError("floor demasiado grande");
    bg.floor.reserve(nFloor);
    for (uint32_t i = 0; i < nFloor; ++i) {
        TileData t;
        t.pos = receivePosition();
        t.id  = static_cast<TileTextureId>(base.receiveUint8());
        bg.floor.push_back(t);
    }
    uint16_t n = base.receiveUint16();
    for (uint16_t i = 0; i < n; ++i) {
        DecorData d;
        d.pos = receivePosition();
        d.id  = static_cast<DecorId>(base.receiveUint8());
        bg.decor.push_back(d);
    }
    n = base.receiveUint16();
    for (uint16_t i = 0; i < n; ++i) {
        BuildingData b;
        b.pos = receivePosition();
        b.id  = static_cast<BuildingId>(base.receiveUint8());
        bg.buildings.push_back(b);
    }
    n = base.receiveUint16();
    for (uint16_t i = 0; i < n; ++i) {
        StaticFurnitureData f;
        f.pos = receivePosition();
        f.id  = static_cast<FurnitureId>(base.receiveUint8());
        f.dir = static_cast<Direction>(base.receiveUint8());
        bg.furniture.push_back(f);
    }
    return bg;
}

void ClientProtocol::recv(ConstMapData& map) {
    map.farm = receiveBackgroundData();
    uint8_t n = base.receiveUint8();
    map.levels.clear();
    for (uint8_t i = 0; i < n; ++i) map.levels.push_back(receiveBackgroundData());
    n = base.receiveUint8();
    map.farm_houses.clear();
    for (uint8_t i = 0; i < n; ++i) map.farm_houses.push_back(receiveBackgroundData());
    map.pierres = receiveBackgroundData();
    map.robins  = receiveBackgroundData();
    map.clints  = receiveBackgroundData();
    map.marnies = receiveBackgroundData();
    map.town    = receiveBackgroundData();
}
