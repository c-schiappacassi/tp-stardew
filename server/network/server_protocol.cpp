#include "server_protocol.h"

#include <algorithm>
#include <cstdint>
#include <set>
#include <limits>
#include <utility>

#include "protocol_errors.h"

namespace {
constexpr size_t MAX_U8 = std::numeric_limits<uint8_t>::max();
constexpr size_t MAX_U16 = std::numeric_limits<uint16_t>::max();

constexpr uint8_t BUNDLE_COUNT = 5;
constexpr uint8_t REQUIREMENT_LIMIT = 10;
constexpr uint8_t SEED_COUNT = 4;

void requireCommunity(bool condition, const char* message) {
    if (!condition) throw ProtocolError(message);
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

// Los contadores viajan en 1 o 2 bytes: si no entran, se rechaza el mensaje
// ANTES de armarlo (si no, el contador se truncaba y el cliente se desincronizaba).
void validateCount(size_t count, size_t limit, const char* what) {
    if (count > limit) {
        throw ProtocolError(std::string("Too many protocol entries: ") + what);
    }
}

void validateFarmCounts(const FarmData& farm) {
    validateCount(farm.crops.size(), MAX_U16, "crops");
    validateCount(farm.animals.size(), MAX_U16, "animals");
    validateCount(farm.tilledSoil.size(), MAX_U16, "tilled soil");
}

void validatePosition(const Position& pos) {
    if (pos.getX() < 0 || pos.getY() < 0 || pos.getX() > static_cast<int>(MAX_U16) ||
        pos.getY() > static_cast<int>(MAX_U16)) {
        throw ProtocolError("Position outside protocol range");
    }
}

void validateLoot(const std::vector<LootData>& loot) {
    validateCount(loot.size(), MAX_U16, "ground items");
    for (const auto& item: loot) {
        validatePosition(item.pos);
        if (item.id == 0 || item.quantity == 0 || item.kind > LootKind::Gold) {
            throw ProtocolError("Invalid ground item");
        }
    }
}

void validateCombat(const DungeonCombatData& combat) {
    validateCount(combat.enemies.size(), MAX_U16, "enemies");
    validateCount(combat.projectiles.size(), MAX_U16, "projectiles");
    validateCount(combat.traps.size(), MAX_U16, "traps");
    validateLoot(combat.loot);
    for (const auto& enemy: combat.enemies) {
        validatePosition(enemy.pos);
        if (enemy.dir > Direction::Right || enemy.type > EnemyType::Skull ||
            enemy.state > EnemyState::Dead) {
            throw ProtocolError("Unknown enemy state");
        }
    }
    for (const auto& projectile: combat.projectiles) {
        validatePosition(projectile.pos);
        if (projectile.type > FlameType::Common) {
            throw ProtocolError("Unknown flame type");
        }
    }
    for (const auto& trap: combat.traps) {
        validatePosition(trap.pos);
        if (trap.dir == Direction::None || trap.dir > Direction::Right ||
            trap.type > TrapType::WallFlame || trap.state > TrapState::Active) {
            throw ProtocolError("Unknown trap state");
        }
    }
}

bool hasMatchingZoneData(const PlayerDataLocal& data) {
    switch (data.currentZone) {
        case Zone::Farm:
            return std::holds_alternative<FarmData>(data.zoneData);
        case Zone::Dungeon:
            return std::holds_alternative<MineData>(data.zoneData);
        case Zone::Town:
            return std::holds_alternative<std::monostate>(data.zoneData);
        case Zone::InsideHouse: {
            const auto* inside = std::get_if<InteriorData>(&data.zoneData);
            return inside && data.interior &&
                   inside->interior_id == data.interior->interior_id &&
                   inside->house_index == data.interior->house_index;
        }
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
        throw ProtocolError("Local data does not match the player zone");
    }

    validateCount(status.players.size(), MAX_U16, "players");
    validateCount(local.items.size(), MAX_U8, "items");
    validateCount(local.newMessages.size(), MAX_U16, "chat messages");
    const auto* farm = std::get_if<FarmData>(&local.zoneData);
    if (farm) {
        validateFarmCounts(*farm);
        validateLoot(farm->loot);
    }
    const auto* mine = std::get_if<MineData>(&local.zoneData);
    if (mine) {
        validateCount(mine->current_level_data.minerals.size(), MAX_U16, "minerals");
        validateCombat(mine->current_level_data.combat);
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
        throw ProtocolError("invalid direction"); // dir fuera de rango corta conexion?
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
    if (data.currentZone == Zone::InsideHouse) {
        if (!data.interior || data.interior->interior_id > InteriorId::ComunityCenter) {
            throw ProtocolError("Unknown interior location");
        }
        out.putUint8(static_cast<uint8_t>(data.interior->interior_id));
        out.putUint8(data.interior->house_index);
    }
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
    for (const MineralData& mineral: level.minerals) {
        writeMineralData(out, mineral);
    }
    writeCombatData(out, level.combat);
}

void ServerProtocol::writeCombatData(MessageBuffer& out, const DungeonCombatData& combat) {
    out.putUint32(combat.visitId);
    out.putUint8(combat.room);
    out.putUint16(static_cast<uint16_t>(combat.enemies.size()));
    for (const auto& enemy: combat.enemies) {
        out.putUint32(enemy.id);
        writePosition(out, enemy.pos);
        out.putUint8(static_cast<uint8_t>(enemy.dir));
        out.putUint8(static_cast<uint8_t>(enemy.type));
        out.putUint8(static_cast<uint8_t>(enemy.state));
        out.putUint16(enemy.health);
        out.putUint16(enemy.maxHealth);
        out.putUint8(enemy.frenzied ? 1 : 0);
        out.putUint32(enemy.action);
    }
    out.putUint16(static_cast<uint16_t>(combat.projectiles.size()));
    for (const auto& projectile: combat.projectiles) {
        out.putUint32(projectile.id);
        writePosition(out, projectile.pos);
        out.putUint8(static_cast<uint8_t>(projectile.type));
    }
    out.putUint16(static_cast<uint16_t>(combat.traps.size()));
    for (const auto& trap: combat.traps) {
        out.putUint32(trap.id);
        writePosition(out, trap.pos);
        out.putUint8(static_cast<uint8_t>(trap.dir));
        out.putUint8(static_cast<uint8_t>(trap.type));
        out.putUint8(static_cast<uint8_t>(trap.state));
        out.putUint16(trap.range);
    }
    writeLootData(out, combat.loot);
}

void ServerProtocol::writeLootData(MessageBuffer& out, const std::vector<LootData>& items) {
    out.putUint16(static_cast<uint16_t>(items.size()));
    for (const auto& loot: items) {
        out.putUint32(loot.id);
        writePosition(out, loot.pos);
        out.putUint16(static_cast<uint16_t>(loot.item));
        out.putUint32(loot.quantity);
        out.putUint8(static_cast<uint8_t>(loot.quality));
        out.putUint8(static_cast<uint8_t>(loot.kind));
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

void ServerProtocol::writeFurnitureData(MessageBuffer& out, const FurnitureData& f) {
    out.putUint32(f.id);
    writePosition(out, f.pos);
    out.putUint8(static_cast<uint8_t>(f.type));
    out.putUint8(static_cast<uint8_t>(f.dir));
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
    out.putUint16(static_cast<uint16_t>(farm.furniture.size()));
    for (const auto& furniture : farm.furniture) {
        writeFurnitureData(out, furniture);
    }
    writeLootData(out, farm.loot);
}

void ServerProtocol::writeBackgroundData(MessageBuffer& out, const BackgroundData& bg) {
    validateCount(bg.decor.size(),     MAX_U16, "decor");
    validateCount(bg.buildings.size(), MAX_U16, "buildings");
    validateCount(bg.furniture.size(), MAX_U16, "static furniture");
    validateCount(bg.portals.size(), MAX_U16, "portals");
    for (const auto& portal: bg.portals) {
        validatePosition(portal.pos);
        if (portal.id > PortalId::MineToFarm) {
            throw ProtocolError("Unknown portal type");
        }
    }

    out.putUint32(static_cast<uint32_t>(bg.floor.size()));
    for (const auto& t : bg.floor) {
        writePosition(out, t.pos);
        out.putUint8(static_cast<uint8_t>(t.id));
    }
    out.putUint16(static_cast<uint16_t>(bg.decor.size()));
    for (const auto& d : bg.decor) {
        writePosition(out, d.pos);
        out.putUint8(static_cast<uint8_t>(d.id));
    }
    out.putUint16(static_cast<uint16_t>(bg.buildings.size()));
    for (const auto& b : bg.buildings) {
        writePosition(out, b.pos);
        out.putUint8(static_cast<uint8_t>(b.id));
    }
    out.putUint16(static_cast<uint16_t>(bg.furniture.size()));
    for (const auto& f : bg.furniture) {
        writePosition(out, f.pos);
        out.putUint8(static_cast<uint8_t>(f.id));
        out.putUint8(static_cast<uint8_t>(f.dir));
    }
    out.putUint16(static_cast<uint16_t>(bg.portals.size()));
    for (const auto& portal: bg.portals) {
        writePosition(out, portal.pos);
        out.putUint8(static_cast<uint8_t>(portal.id));
    }
    out.putUint8(bg.interior ? 1 : 0);
    if (bg.interior) {
        validatePosition(bg.interior->pos);
        if (bg.interior->interior_id > InteriorId::ComunityCenter) {
            throw ProtocolError("Unknown interior");
        }
        writePosition(out, bg.interior->pos);
        out.putUint8(static_cast<uint8_t>(bg.interior->interior_id));
        out.putUint8(bg.interior->npc ? 1 : 0);
        if (bg.interior->npc) {
            writePosition(out, bg.interior->npc->pos);
            out.putUint8(static_cast<uint8_t>(bg.interior->npc->id));
        }
    }
}

void ServerProtocol::sendConstMap(const ConstMapData& map) {
    validateCount(map.levels.size(),      MAX_U8, "levels");
    validateCount(map.farm_houses.size(), MAX_U8, "houses");

    MessageBuffer out(1 << 20);
    writeBackgroundData(out, map.farm);
    out.putUint8(static_cast<uint8_t>(map.levels.size()));
    for (const auto& l : map.levels) writeBackgroundData(out, l);
    out.putUint8(static_cast<uint8_t>(map.farm_houses.size()));
    for (const auto& h : map.farm_houses) writeBackgroundData(out, h);
    writeBackgroundData(out, map.pierres);
    writeBackgroundData(out, map.robins);
    writeBackgroundData(out, map.clints);
    writeBackgroundData(out, map.marnies);
    writeBackgroundData(out, map.community_center);
    writeBackgroundData(out, map.town);
    base.send(out);
}

void ServerProtocol::writeZoneData(MessageBuffer& out, Zone currentZone,const ZoneData& zoneData) {
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
        case Zone::InsideHouse: {
            const auto* inside = std::get_if<InteriorData>(&zoneData);
            if (!inside) throw ProtocolError("Missing interior data");
            validateCount(inside->furniture.size(), MAX_U16, "interior furniture");
            out.putUint8(static_cast<uint8_t>(inside->interior_id));
            out.putUint8(inside->house_index);
            out.putUint16(static_cast<uint16_t>(inside->furniture.size()));
            for (const auto& furniture: inside->furniture) writeFurnitureData(out, furniture);
            return;
        }
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
    out.putUint8(localData.cap_inventory);
    out.putUint32(localData.gold);
    out.putUint16(localData.health);
    out.putUint16(localData.maxHealth);
    out.putUint16(localData.energy);
    out.putUint16(localData.maxEnergy);
    writeZoneData(out, localData.currentZone, localData.zoneData);
    out.putUint8(localData.combatProgress.skeletons ? 1 : 0);
    out.putUint8(localData.combatProgress.vampires ? 1 : 0);
    out.putUint8(localData.combatProgress.orcs ? 1 : 0);
    out.putUint8(localData.combatProgress.skull ? 1 : 0);
    out.putUint16(static_cast<uint16_t>(localData.newMessages.size()));
    for (const auto& message : localData.newMessages) {
        out.putString(message.sender);
        out.putString(message.text);
    }
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
    dto.mapPath = base.receiveString();
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

DayOkDTO ServerProtocol::receiveDayOk() { return DayOkDTO{base.receiveString()}; }

DayCancelDTO ServerProtocol::receiveDayCancel() { return DayCancelDTO{base.receiveString()}; }

DonateDTO ServerProtocol::receiveDonate() {
    DonateDTO command(base.receiveString());
    command.kind = static_cast<DonationKind>(base.receiveUint8());
    validateDonationKind(command.kind);
    command.slot = base.receiveUint8();
    command.quantity = base.receiveUint32();
    command.requestId = base.receiveUint32();
    return command;
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
    out.putUint8(status.dayEnded ? 1 : 0);
    out.putUint8(status.dayConfirmationRequired ? 1 : 0);
    out.putUint16(status.timeOfDayMinutes);
    out.putUint8(status.gameCompleted ? 1 : 0);
    writeCommunityState(out, status.community);
    base.send(out);
}

void ServerProtocol::sendQtStatus(const qtStatus& status) {
    validateCount(status.games.size(), MAX_U16, "games");

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

void ServerProtocol::writeCommunityStacks(MessageBuffer& out, const std::vector<Stack>& items) {
    out.putUint8(static_cast<uint8_t>(items.size()));
    for (const auto& item : items) writeStack(out, item);
}

void ServerProtocol::writeCommunityState(MessageBuffer& out, const CommunityStateData& state) {
    validateCommunityState(state);
    out.putUint8(state.available);
    out.putUint8(state.insideCenter);
    out.putUint8(state.canDonate);
    out.putUint8(state.completed);
    out.putUint8(static_cast<uint8_t>(state.discount));
    out.putUint8(static_cast<uint8_t>(state.bundles.size()));
    for (const auto& bundle : state.bundles) {
        out.putUint8(static_cast<uint8_t>(bundle.id));
        out.putUint8(bundle.completed);
        out.putUint8(bundle.rewardGranted);
        out.putUint8(static_cast<uint8_t>(bundle.items.size()));
        for (const auto& item : bundle.items) {
            out.putUint16(static_cast<uint16_t>(item.item));
            out.putUint32(item.donated);
            out.putUint32(item.required);
            out.putUint8(static_cast<uint8_t>(item.minimumQuality));
        }
        writeCommunityStacks(out, bundle.reward.items);
        out.putUint32(bundle.reward.coins);
        out.putUint8(bundle.reward.cows);
        out.putUint8(bundle.reward.chickens);
        out.putUint8(bundle.reward.statsBonusPercent);
        out.putUint8(bundle.reward.marketDiscountNextDay);
    }
    out.putUint32(state.coinsDonated);
    out.putUint32(state.coinsRequired);
    out.putUint8(state.combat.skeletons);
    out.putUint8(state.combat.vampires);
    out.putUint8(state.combat.orcs);
    out.putUint8(state.combat.skull);
    writeCommunityStacks(out, state.pendingItems);
    out.putUint32(static_cast<uint32_t>(state.pendingCoins >> 32));
    out.putUint32(static_cast<uint32_t>(state.pendingCoins));
    out.putUint8(state.lastDonation.has_value());
    if (state.lastDonation) {
        const auto& receipt = *state.lastDonation;
        out.putUint32(receipt.requestId);
        out.putUint8(static_cast<uint8_t>(receipt.kind));
        out.putUint8(receipt.slot);
        out.putUint8(static_cast<uint8_t>(receipt.error));
        out.putUint32(receipt.accepted);
    }
}
