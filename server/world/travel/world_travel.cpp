#include "world_travel.h"

#include "dungeon/contracts/dungeon_entrance.h"
#include "dungeon/dungeon_manager.h"
#include "farm/farm_manager.h"
#include "farm/loading/farm_layout.h"
#include "player/player_manager.h"
#include "town/town_map.h"
#include "world/interiors/interior_rooms.h"

WorldTravel::WorldTravel(DungeonManager& dungeon, const FarmManager& farm, const TownMap& town, const InteriorRooms& interiors, EntityIdGenerator& ids):
    dungeon(dungeon), farm(farm), town(town), interiors(interiors), ids(ids) {}

void WorldTravel::configureFarm(const FarmLayout& layout) {
    resetDungeon();
    farmTown.reset();
    portals.clear();
    for (const auto& room: layout.interiors) {
        const WorldLocation inside{Zone::InsideHouse, 0, room.location, room.entry};
        auto exit = inside;
        exit.position = room.exit;
        auto outsideReturn = room.outside;
        outsideReturn.position = room.returnPosition;
        portals.push_back({room.outside, inside});
        for (const auto& position: room.extraOutside) {
            auto extra = room.outside;
            extra.position = position;
            portals.push_back({extra, inside});
        }
        portals.push_back({exit, outsideReturn});
    }
    if (layout.townDoor)
        farmTown = FarmTownPassage{*layout.townDoor, layout.townReturn.value_or(layout.spawn), layout.extraTownDoors};
}

void WorldTravel::resetDungeon() {
    entrance.reset();
    mineReturns.clear();
}

bool WorldTravel::configureEntrance(const DungeonEntrance& data) {
    const CollisionBox playerBox{entity_dimensions::PLAYER};
    if ((data.zone != Zone::Farm && data.zone != Zone::Town) || !data.terrain.isWalkable(playerBox.at(data.position)) ||
        !data.terrain.isWalkable(playerBox.at(data.returnPosition)))
        return false;
    if (data.zone == Zone::Farm && (!farm.isInitialized() || !farm.canWalk(playerBox.at(data.position)) || !farm.canWalk(playerBox.at(data.returnPosition))))
        return false;
    if (data.zone == Zone::Town && (!town.canWalk(playerBox.at(data.position)) || !town.canWalk(playerBox.at(data.returnPosition))))
        return false;
    entrance = Entrance{data.zone, data.position, data.returnPosition};
    return true;
}

bool WorldTravel::canConnectTown(const TownMap& candidate) const {
    if (!farm.isInitialized() || !farmTown || !candidate.connectionTo(Zone::Farm))
        return false;
    const CollisionBox playerBox{entity_dimensions::PLAYER};
    return farm.canWalk(playerBox.at(farmTown->door)) && farm.canWalk(playerBox.at(farmTown->returnPosition));
}

std::optional<Position> WorldTravel::getDungeonEntrance(Zone zone) const {
    return entrance && entrance->zone == zone ? std::optional<Position>{entrance->position} : std::nullopt;
}

bool WorldTravel::canArrive(const PlayerSnapshot& player, const ReturnPoint& target) const {
    const auto bounds = player.getCollisionBounds(target.position);
    if (target.zone == Zone::Farm)
        return farm.isInitialized() && farm.canWalk(bounds);
    return target.zone == Zone::Town && town.canWalk(bounds);
}

bool WorldTravel::sameBaseCell(const PlayerSnapshot& player, const WorldLocation& location) const {
    if (location.zone == Zone::Farm)
        return farm.sameBaseCell(player.getPosition(), location.position);
    if (location.zone == Zone::Town)
        return town.getMap().sameBaseCell(player.getPosition(), location.position);
    const auto* room = location.zone == Zone::InsideHouse && location.interior ? interiors.find(*location.interior) : nullptr;
    return room && room->sameBaseCell(player.getPosition(), location.position);
}

bool WorldTravel::enterMine(PlayerManager& players, uint16_t playerId, const ReturnPoint& origin) {
    if (!dungeon.enter(players, playerId, ids))
        return false;
    mineReturns.insert_or_assign(playerId, origin);
    return true;
}

std::optional<bool> WorldTravel::usePortal(PlayerManager& players, uint16_t playerId, const PlayerSnapshot& player) {
    for (const auto& portal: portals) {
        const auto& source = portal.source;
        if (source.zone != player.getZone() || source.level != player.getDungeonLevel() || source.interior != player.getInterior() || !sameBaseCell(player, source))
            continue;
        const auto& target = portal.destination;
        const bool canWalk = target.zone == Zone::InsideHouse && target.interior ?
            interiors.canWalk(*target.interior, player.getCollisionBounds(target.position)) : canArrive(player, {target.zone, target.position});
        return canWalk && players.apply(playerId, player_action::ChangeLocation{target.zone, target.level, target.position, target.interior}).applied;
    }
    return std::nullopt;
}

bool WorldTravel::useMinePassage(PlayerManager& players, uint16_t playerId, const PlayerSnapshot& player) {
    if (!dungeon.canExit(player))
        return dungeon.advanceRoom(players, playerId);
    const auto found = mineReturns.find(playerId);
    if (found == mineReturns.end() && !entrance)
        return false;
    const ReturnPoint target = found != mineReturns.end() ? found->second : ReturnPoint{entrance->zone, entrance->returnPosition};
    if (!canArrive(player, target) || !players.apply(playerId, player_action::ChangeLocation{target.zone, 0, target.position}).applied)
        return false;
    mineReturns.erase(playerId);
    return true;
}

std::optional<bool> WorldTravel::useTownPassage(PlayerManager& players, uint16_t playerId, const PlayerSnapshot& player) {
    const auto townFarmDoor = town.connectionTo(Zone::Farm);
    if (player.getZone() == Zone::Farm && farmTown && townFarmDoor && farm.sameBaseCell(player.getPosition(), farmTown->door)) {
        const ReturnPoint target{Zone::Town, *townFarmDoor};
        return canArrive(player, target) && players.apply(playerId, player_action::ChangeLocation{target.zone, 0, target.position}).applied;
    }
    if (player.getZone() != Zone::Town)
        return std::nullopt;
    if (townFarmDoor && farmTown && town.isFarmDoor(player.getPosition())) {
        const ReturnPoint target{Zone::Farm, farmTown->returnPosition};
        return canArrive(player, target) && players.apply(playerId, player_action::ChangeLocation{target.zone, 0, target.position}).applied;
    }
    const auto townMineDoor = town.connectionTo(Zone::Dungeon);
    if (townMineDoor && town.getMap().sameBaseCell(player.getPosition(), *townMineDoor))
        return enterMine(players, playerId, {Zone::Town, *townMineDoor});
    return std::nullopt;
}

bool WorldTravel::useEntrance(PlayerManager& players, uint16_t playerId, const PlayerSnapshot& player) {
    if (!entrance || player.getZone() != entrance->zone)
        return false;
    const bool atEntrance = entrance->zone == Zone::Farm ? farm.sameBaseCell(player.getPosition(), entrance->position) :
        town.getMap().sameBaseCell(player.getPosition(), entrance->position);
    return atEntrance && enterMine(players, playerId, {entrance->zone, entrance->returnPosition});
}

bool WorldTravel::use(PlayerManager& players, uint16_t playerId) {
    const auto player = players.snapshot(playerId);
    if (!player.canAct())
        return false;
    auto moved = usePortal(players, playerId, player);
    if (!moved && player.getZone() == Zone::Dungeon)
        moved = useMinePassage(players, playerId, player);
    if (!moved)
        moved = useTownPassage(players, playerId, player);
    if (!moved)
        moved = useEntrance(players, playerId, player);
    if (*moved)
        notifyTravel(playerId, player, players.snapshot(playerId));
    return *moved;
}

bool WorldTravel::teleportTo(PlayerManager& players, uint16_t playerId, const PlayerSnapshot& player, Zone destination, uint8_t level, std::optional<InteriorLocation> inside) {
    if (destination == Zone::Dungeon)
        return dungeon.teleportToRoom(players, playerId, level, ids);
    if (destination == Zone::Farm) {
        const auto spawn = farm.spawnFor(player);
        return spawn && players.apply(playerId, player_action::RestoreLocation{spawn->zone, spawn->level, spawn->position, spawn->interior}).applied;
    }
    if (destination == Zone::Town) {
        const auto entry = town.connectionTo(Zone::Farm);
        return entry && town.canWalk(player.getCollisionBounds(*entry)) && players.apply(playerId, player_action::ChangeLocation{Zone::Town, 0, *entry}).applied;
    }
    const auto* room = destination == Zone::InsideHouse && inside ? interiors.find(*inside) : nullptr;
    return room && room->canWalk(player.getCollisionBounds(room->getEntry())) &&
        players.apply(playerId, player_action::ChangeLocation{Zone::InsideHouse, 0, room->getEntry(), inside}).applied;
}

bool WorldTravel::teleport(PlayerManager& players, uint16_t playerId, Zone destination, uint8_t level, std::optional<InteriorLocation> inside) {
    const auto origin = players.snapshot(playerId);
    if (!origin.canAct() || !teleportTo(players, playerId, origin, destination, level, inside))
        return false;
    const auto arrived = players.snapshot(playerId);
    onTeleport(playerId, origin, arrived);
    notifyTravel(playerId, origin, arrived);
    return true;
}

void WorldTravel::notifyTravel(uint16_t playerId, const PlayerSnapshot& origin, const PlayerSnapshot& arrived) {
    if (origin.getZone() == Zone::Dungeon || arrived.getZone() == Zone::Dungeon)
        dungeon.onTravel(playerId, arrived.getDungeonLevel(), arrived.canBeAttacked());
}

void WorldTravel::finishDay() { mineReturns.clear(); }

void WorldTravel::onTeleport(uint16_t playerId, const PlayerSnapshot& origin, const PlayerSnapshot& destination) {
    if (destination.getZone() != Zone::Dungeon) {
        mineReturns.erase(playerId);
    } else if (origin.getZone() != Zone::Dungeon) {
        if (origin.getZone() == Zone::Farm || origin.getZone() == Zone::Town) {
            mineReturns.insert_or_assign(playerId, ReturnPoint{origin.getZone(), origin.getPosition()});
        } else if (entrance) {
            mineReturns.insert_or_assign(playerId, ReturnPoint{entrance->zone, entrance->returnPosition});
        } else if (const auto outside = farm.spawnFor(origin)) {
            mineReturns.insert_or_assign(playerId, ReturnPoint{Zone::Farm, outside->position});
        }
    }
}
