#include "client_protocol.h"

#include <algorithm>
#include <cstdint>
#include <set>
#include <stdexcept>
#include <utility>

#include "constants.h"
#include "furniture_data.h"
#include "protocol_errors.h"

namespace {
constexpr uint8_t BUNDLE_COUNT = 5;
constexpr uint8_t REQUIREMENT_LIMIT = 10;
constexpr uint8_t SEED_COUNT = 4;

void requireCommunity(bool condition, const char* message) {
    if (!condition) throw ProtocolError(message);
}

bool receiveCommunityFlag(ProtocolBase& in) {
    const auto value = in.receiveUint8();
    requireCommunity(value <= 1, "Invalid community flag");
    return value != 0;
}

uint8_t receiveCommunityCount(ProtocolBase& in, uint8_t limit) {
    const auto value = in.receiveUint8();
    requireCommunity(value <= limit, "Too many community entries");
    return value;
}

void validateCommunityQuality(Quality quality) {
    switch (quality) {
        case Quality::None:
        case Quality::Base:
        case Quality::Good:
        case Quality::Excellent:
        case Quality::Incredible:
        case Quality::Perfect:
            return;
    }
    throw ProtocolError("Invalid community item quality");
}

void validateCommunityStacks(const std::vector<Stack>& items) {
    requireCommunity(items.size() <= SEED_COUNT, "Too many community reward items");
    for (const auto& stack : items) {
        requireCommunity(stack.quantity != 0, "Empty community reward stack");
        validateCommunityQuality(stack.qual);
    }
}

void validateDonationKind(DonationKind kind) {
    requireCommunity(kind <= DonationKind::Coins, "Invalid donation kind");
}

void validateCommunityState(const CommunityStateData& state) {
    requireCommunity(state.bundles.size() <= BUNDLE_COUNT, "Too many community bundles");
    requireCommunity(state.discount <= MarketDiscount::Active, "Invalid community discount");
    requireCommunity(!state.canDonate || (state.insideCenter && !state.completed), "Invalid donation availability");
    requireCommunity(state.coinsDonated <= state.coinsRequired, "Invalid community coins");
    if (state.available) {
        requireCommunity(state.bundles.size() == BUNDLE_COUNT && state.coinsRequired != 0,
                "Incomplete community state");
    }
    std::set<CommunityBundle> seen;
    for (const auto& bundle : state.bundles) {
        requireCommunity(bundle.id <= CommunityBundle::Money && seen.insert(bundle.id).second,
                "Invalid community bundle");
        requireCommunity(!bundle.rewardGranted || bundle.completed, "Reward granted for an incomplete bundle");
        requireCommunity(bundle.items.size() <= REQUIREMENT_LIMIT, "Too many community requirements");
        for (const auto& item : bundle.items) {
            requireCommunity(item.required != 0 && item.donated <= item.required, "Invalid community requirement");
            validateCommunityQuality(item.minimumQuality);
        }
        validateCommunityStacks(bundle.reward.items);
    }
    if (state.completed) {
        requireCommunity(state.available && std::all_of(state.bundles.begin(), state.bundles.end(),
            [](const auto& bundle) { return bundle.completed; }), "Invalid completed community state");
    }
    validateCommunityStacks(state.pendingItems);
    if (state.lastDonation) {
        const auto& receipt = *state.lastDonation;
        validateDonationKind(receipt.kind);
        requireCommunity(receipt.error <= CommunityDonationError::InvalidSlot, "Invalid donation result");
        requireCommunity(receipt.error == CommunityDonationError::None ? receipt.accepted != 0 : receipt.accepted == 0,
                "Invalid accepted donation amount");
    }
}
}


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
    mineral.health = base.receiveUint16();
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
    loot.quantity = base.receiveUint32();
    loot.quality = static_cast<Quality>(base.receiveUint8());
    loot.kind = static_cast<LootKind>(base.receiveUint8());
    if (loot.id == 0 || loot.quantity == 0 || loot.kind > LootKind::Gold) {
        throw ProtocolError("Invalid ground item");
    }
    return loot;
}

std::vector<LootData> ClientProtocol::receiveLoot() {
    const auto count = base.receiveUint16();
    std::vector<LootData> loot;
    loot.reserve(count);
    for (uint16_t i = 0; i < count; ++i) {
        loot.push_back(receiveLootData());
    }
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
    combat.loot = receiveLoot();
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
    level.furniture = receiveFurniture();
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
    if (data.currentZone > Zone::InsideHouse) throw ProtocolError("Unknown zone");
    if (data.currentZone == Zone::Dungeon) {
        data.dungeonLevel = base.receiveUint8();
    }
    if (data.currentZone == Zone::InsideHouse) {
        InteriorLocation inside;
        inside.interior_id = static_cast<InteriorId>(base.receiveUint8());
        inside.house_index = base.receiveUint8();
        if (inside.interior_id > InteriorId::ComunityCenter) {
            throw ProtocolError("Unknown interior");
        }
        data.interior = inside;
    }
    const uint16_t heldItem = base.receiveUint16();
    if (heldItem != 0) data.heldItem = static_cast<ItemId>(heldItem);
    data.health = base.receiveUint16();
    data.maxHealth = base.receiveUint16();
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
    localData.cap_inventory = base.receiveUint8();
    localData.gold = base.receiveUint32();
    localData.energy = base.receiveUint16();
    localData.maxEnergy = base.receiveUint16();
    localData.zoneData = receiveZoneData(baseData.currentZone);
    localData.combatProgress.skeletons = base.receiveUint8() != 0;
    localData.combatProgress.vampires = base.receiveUint8() != 0;
    localData.combatProgress.orcs = base.receiveUint8() != 0;
    localData.combatProgress.skull = base.receiveUint8() != 0;

    const uint16_t messageCount = base.receiveUint16();
    localData.newMessages.reserve(messageCount);
    for (uint16_t i = 0; i < messageCount; ++i) {
        ChatMessageData message;
        message.sender = base.receiveString();
        message.text = base.receiveString();
        localData.newMessages.push_back(std::move(message));
    }
    localData.openChest = receiveOpenChest();
    if (localData.openChest) {
        if (!hasChest(localData.zoneData, localData.openChest->id)) {
            throw ProtocolError("Open chest does not belong to the player zone");
        }
    }
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
    farm.furniture = receiveFurniture();
    farm.loot = receiveLoot();
    return farm;
}

FurnitureData ClientProtocol::receiveFurnitureData() {
    FurnitureData f;
    f.id = base.receiveUint32();
    f.pos = receivePosition();
    f.type = static_cast<FurnitureId>(base.receiveUint8());
    f.dir = static_cast<Direction>(base.receiveUint8());
    if (f.id == 0 || f.type > FurnitureId::Chest || f.dir > Direction::Right)
        throw ProtocolError("Invalid furniture");
    return f;
}

std::vector<FurnitureData> ClientProtocol::receiveFurniture() {
    const auto count = base.receiveUint16();
    std::vector<FurnitureData> result;
    std::set<uint32_t> ids;
    result.reserve(count);
    for (uint16_t i = 0; i < count; ++i) {
        auto item = receiveFurnitureData();
        if (!ids.insert(item.id).second)
            throw ProtocolError("Duplicate furniture identifier");
        result.push_back(item);
    }
    return result;
}

InteriorData ClientProtocol::receiveInteriorData() {
    InteriorData in;
    in.interior_id = static_cast<InteriorId>(base.receiveUint8());
    if (in.interior_id > InteriorId::ComunityCenter) {
        throw ProtocolError("Unknown interior");
    }
    in.house_index = base.receiveUint8();
    in.furniture = receiveFurniture();
    in.loot = receiveLoot();
    return in;
}

ZoneData ClientProtocol::receiveZoneData(Zone currentZone) {
    switch (currentZone) {
        case Zone::Dungeon: return ZoneData{receiveMineData()};
        case Zone::Farm:    return ZoneData{receiveFarmData()};
        case Zone::Town:    return ZoneData{std::monostate{}};
        case Zone::InsideHouse: return ZoneData{receiveInteriorData()};
    }
    throw ProtocolError("Unknown ZoneData");
}

// SENDERS

void ClientProtocol::send(const CommandDTO& dto) {
    switch (dto.type()) {
        case MessageType::CreateGame: send(static_cast<const CreateGameDTO&>(dto)); break;
        case MessageType::JoinGame: send(static_cast<const JoinGameDTO&>(dto)); break;
        case MessageType::ResumeGame: send(static_cast<const ResumeGameDTO&>(dto)); break;
        case MessageType::Move: send(static_cast<const MoveDTO&>(dto)); break;
        case MessageType::Interact: send(static_cast<const InteractDTO&>(dto)); break;
        case MessageType::ItemTransfer:
            send(static_cast<const ItemTransferDTO&>(dto));
            break;
        case MessageType::SelectItem:
            send(static_cast<const SelectItemDTO&>(dto));
            break;
        case MessageType::CloseChest:
            send(static_cast<const CloseChestDTO&>(dto));
            break;
        case MessageType::DayOk:send(static_cast<const DayOkDTO&>(dto));break;
        case MessageType::DayCancel:send(static_cast<const DayCancelDTO&>(dto));break;
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
    out.putString(dto.mapPath);
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
    validateDonationKind(dto.kind);
    out.putString(dto.userName);
    out.putUint8(static_cast<uint8_t>(dto.kind));
    out.putUint8(dto.slot);
    out.putUint32(dto.quantity);
    out.putUint32(dto.requestId);
    base.send(out);
}

void ClientProtocol::send(const DayOkDTO& dto) {
    MessageBuffer out;
    out.putUint8(static_cast<uint8_t>(MessageType::DayOk));
    out.putString(dto.userName);
    base.send(out);
}

void ClientProtocol::send(const DayCancelDTO& dto) {
    MessageBuffer out;
    out.putUint8(static_cast<uint8_t>(MessageType::DayCancel));
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

void ClientProtocol::send(const ItemTransferDTO& dto) {
    MessageBuffer out;
    out.putUint8(static_cast<uint8_t>(MessageType::ItemTransfer));
    out.putString(dto.userName);
    out.putUint32(dto.chestId);
    out.putUint8(dto.slot);
    out.putUint16(dto.quantity);
    base.send(out);
}

void ClientProtocol::send(const SelectItemDTO& dto) {
    MessageBuffer out;
    out.putUint8(static_cast<uint8_t>(MessageType::SelectItem));
    out.putString(dto.userName);
    out.putUint32(dto.chestId);
    out.putUint8(dto.slot);
    out.putUint16(dto.quantity);
    base.send(out);
}

void ClientProtocol::send(const CloseChestDTO& dto) {
    MessageBuffer out;
    out.putUint8(static_cast<uint8_t>(MessageType::CloseChest));
    out.putString(dto.userName);
    base.send(out);
}

std::optional<OpenChestData> ClientProtocol::receiveOpenChest() {
    const auto present = base.receiveUint8();
    if (present > 1)
        throw ProtocolError("Invalid open chest flag");
    if (present == 0)
        return std::nullopt;
    OpenChestData chest;
    chest.id = base.receiveUint32();
    chest.capacity = base.receiveUint8();
    const auto count = base.receiveUint8();
    if (chest.id == 0 || chest.capacity != CHEST_INVENTORY_CAPACITY || count > chest.capacity) {
        throw ProtocolError("Invalid open chest");
    }
    for (uint8_t i = 0; i < count; ++i) {
        const auto slot = base.receiveUint8();
        const auto stack = receiveStack();
        if (slot >= chest.capacity || stack.quantity == 0 || stack.quantity > INVENTORY_STACK_LIMIT || stack.qual > Quality::Perfect || !chest.items.emplace(slot, stack).second) {
            throw ProtocolError("Invalid chest slot");
        }
    }
    return chest;
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
//                players, localPlayer, dayEnded, dayConfirmationRequired, timeOfDayMinutes,
//                gameCompleted, community }

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
    status.dayConfirmationRequired = base.receiveUint8() != 0;
    status.timeOfDayMinutes = base.receiveUint16();
    status.gameCompleted = base.receiveUint8() != 0;
    status.community = receiveCommunityState();
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


NpcData ClientProtocol::receiveNpcData() {
    const Position pos = receivePosition();
    const auto id = static_cast<NpcId>(base.receiveUint8());
    if (id > NpcId::Clint) throw ProtocolError("Unknown NPC");
    return NpcData{pos, id};
}

InteriorBackgroundData ClientProtocol::receiveInteriorBackground() {
    const Position pos = receivePosition();
    const auto id = static_cast<InteriorId>(base.receiveUint8());
    if (id > InteriorId::ComunityCenter) throw ProtocolError("Unknown interior");

    InteriorBackgroundData interior{pos, id, std::nullopt};
    if (base.receiveUint8() != 0) interior.npc = receiveNpcData();
    return interior;
}

BackgroundData ClientProtocol::receiveBackgroundData() {
    constexpr uint32_t MAX_FLOOR = 1024 * 1024;
    BackgroundData bg;

    const uint32_t nFloor = base.receiveUint32();
    if (nFloor > MAX_FLOOR) throw ProtocolError("floor too large");
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
    bg.frame.reserve(n);
    for (uint16_t i = 0; i < n; ++i) {
        FramePieceData piece;
        piece.pos = receivePosition();
        piece.id = static_cast<FramePieceId>(base.receiveUint8());
        if (piece.id > FramePieceId::DungeonTopWallEndLeft) {
            throw ProtocolError("Unknown frame piece");
        }
        bg.frame.push_back(piece);
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
    n = base.receiveUint16();
    bg.portals.reserve(n);
    for (uint16_t i = 0; i < n; ++i) {
        PortalData portal;
        portal.pos = receivePosition();
        portal.id = static_cast<PortalId>(base.receiveUint8());
        if (portal.id > PortalId::MineToFarm) {
            throw ProtocolError("Unknown portal type");
        }
        bg.portals.push_back(portal);
    }
    if (base.receiveUint8() != 0) bg.interior = receiveInteriorBackground();
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
    map.community_center = receiveBackgroundData();
    map.town    = receiveBackgroundData();
}

std::vector<Stack> ClientProtocol::receiveCommunityStacks() {
    const auto count = receiveCommunityCount(base, SEED_COUNT);
    std::vector<Stack> items;
    for (uint8_t i = 0; i < count; ++i) items.push_back(receiveStack());
    return items;
}

CommunityStateData ClientProtocol::receiveCommunityState() {
    CommunityStateData state;
    state.available = receiveCommunityFlag(base);
    state.insideCenter = receiveCommunityFlag(base);
    state.canDonate = receiveCommunityFlag(base);
    state.completed = receiveCommunityFlag(base);
    state.discount = static_cast<MarketDiscount>(base.receiveUint8());
    const auto count = receiveCommunityCount(base, BUNDLE_COUNT);
    for (uint8_t i = 0; i < count; ++i) {
        CommunityBundleData bundle;
        bundle.id = static_cast<CommunityBundle>(base.receiveUint8());
        bundle.completed = receiveCommunityFlag(base);
        bundle.rewardGranted = receiveCommunityFlag(base);
        const auto items = receiveCommunityCount(base, REQUIREMENT_LIMIT);
        for (uint8_t item = 0; item < items; ++item) {
            const auto id = static_cast<ItemId>(base.receiveUint16());
            const auto donated = base.receiveUint32();
            const auto required = base.receiveUint32();
            const auto quality = static_cast<Quality>(base.receiveUint8());
            bundle.items.push_back({id, donated, required, quality});
        }
        bundle.reward.items = receiveCommunityStacks();
        bundle.reward.coins = base.receiveUint32();
        bundle.reward.cows = base.receiveUint8();
        bundle.reward.chickens = base.receiveUint8();
        bundle.reward.statsBonusPercent = base.receiveUint8();
        bundle.reward.marketDiscountNextDay = receiveCommunityFlag(base);
        state.bundles.push_back(std::move(bundle));
    }
    state.coinsDonated = base.receiveUint32();
    state.coinsRequired = base.receiveUint32();
    state.combat = {receiveCommunityFlag(base), receiveCommunityFlag(base),
                    receiveCommunityFlag(base), receiveCommunityFlag(base)};
    state.pendingItems = receiveCommunityStacks();
    const auto high = base.receiveUint32();
    state.pendingCoins = (static_cast<uint64_t>(high) << 32) | base.receiveUint32();
    if (receiveCommunityFlag(base)) {
        CommunityDonationReceipt receipt;
        receipt.requestId = base.receiveUint32();
        receipt.kind = static_cast<DonationKind>(base.receiveUint8());
        receipt.slot = base.receiveUint8();
        receipt.error = static_cast<CommunityDonationError>(base.receiveUint8());
        receipt.accepted = base.receiveUint32();
        state.lastDonation = receipt;
    }
    validateCommunityState(state);
    return state;
}
