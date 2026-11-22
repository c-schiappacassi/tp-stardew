#include "world_travel.h"

#include "dungeon/dungeon_entrance.h"
#include "dungeon/dungeon_manager.h"
#include "farm/farm_manager.h"
#include "farm/loading/farm_layout.h"
#include "interiors/interior_manager.h"
#include "player/player_manager.h"
#include "town/town_map.h"

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
        portals.push_back({exit, outsideReturn});
    }
    if (layout.townDoor) {
        farmTown = FarmTownPassage{*layout.townDoor, layout.townReturn.value_or(layout.spawn)};
    }
}

void WorldTravel::resetDungeon() {
    entrance.reset();
    mineReturns.clear();
}

bool WorldTravel::configureEntrance(const DungeonEntrance& data, const FarmManager& farm, const TownMap& town) {
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

bool WorldTravel::canConnectTown(const TownMap& town, const FarmManager& farm) const {
    if (!farm.isInitialized() || !farmTown || !town.connectionTo(Zone::Farm)) {
        return false;
    }
    const CollisionBox playerBox{entity_dimensions::PLAYER};
    return farm.canWalk(playerBox.at(farmTown->door)) && farm.canWalk(playerBox.at(farmTown->returnPosition));
}

std::optional<Position> WorldTravel::getDungeonEntrance(Zone zone) const {
    if (!entrance || entrance->zone != zone) {
        return std::nullopt;
    }
    return entrance->position;
}

bool WorldTravel::canArrive(const PlayerSnapshot& player, const ReturnPoint& target, const FarmManager& farm, const TownMap& town) const {
    const auto bounds = player.getCollisionBounds(target.position);
    if (target.zone == Zone::Farm)
        return farm.isInitialized() && farm.canWalk(bounds);
    return target.zone == Zone::Town && town.canWalk(bounds);
}

bool WorldTravel::enterMine(PlayerManager& players, uint16_t playerId, const ReturnPoint& origin, DungeonManager& dungeon) {
    if (!dungeon.enter(players, playerId))
        return false;
    mineReturns.insert_or_assign(playerId, origin);
    return true;
}

bool WorldTravel::use(PlayerManager& players, uint16_t playerId, DungeonManager& dungeon, const FarmManager& farm, const TownMap& town, const InteriorManager& interiors) {
    const auto player = players.snapshot(playerId);
    if (!player.canAct())
        return false;
    const auto sameBaseCell = [&](const WorldLocation& location) {
        if (location.zone == Zone::Farm)
            return farm.sameBaseCell(player.getPosition(), location.position);
        if (location.zone == Zone::Town)
            return town.getMap().sameBaseCell(player.getPosition(), location.position);
        if (location.zone == Zone::InsideHouse && location.interior) {
            const auto* room = interiors.find(*location.interior);
            return room && room->terrain.sameBaseCell(player.getPosition(), location.position);
        }
        return false;
    };
    const auto hasTerrain = [&](const WorldLocation& location) {
        return location.zone == Zone::Farm || location.zone == Zone::Town ||
            (location.zone == Zone::InsideHouse && location.interior && interiors.find(*location.interior));
    };
    for (const auto& portal: portals) {
        const auto& source = portal.source;
        if (source.zone != player.getZone() || source.level != player.getDungeonLevel() || source.interior != player.getInterior() || !sameBaseCell(source))
            continue;
        const auto& target = portal.destination;
        const auto bounds = player.getCollisionBounds(target.position);
        const bool canWalk = target.zone == Zone::InsideHouse && target.interior ?
            interiors.canWalk(*target.interior, bounds) : canArrive(player, {target.zone, target.position}, farm, town);
        return hasTerrain(target) && canWalk && players.apply(playerId, player_action::ChangeLocation{target.zone, target.level, target.position, target.interior}).applied;
    }
    const auto townFarmDoor = town.connectionTo(Zone::Farm);
    const auto townMineDoor = town.connectionTo(Zone::Dungeon);
    if (player.getZone() == Zone::Dungeon) {
        if (!dungeon.canExit(player))
            return dungeon.advanceRoom(players, playerId);
        const auto found = mineReturns.find(playerId);
        if (found == mineReturns.end() && !entrance)
            return false;
        const ReturnPoint target = found != mineReturns.end() ? found->second : ReturnPoint{entrance->zone, entrance->returnPosition};
        if (!canArrive(player, target, farm, town) || !players.apply(playerId, player_action::ChangeLocation{target.zone, 0, target.position}).applied)
            return false;
        mineReturns.erase(playerId);
        return true;
    }
    if (player.getZone() == Zone::Farm && farmTown && townFarmDoor && farm.sameBaseCell(player.getPosition(), farmTown->door)) {
        const ReturnPoint target{Zone::Town, *townFarmDoor};
        return canArrive(player, target, farm, town) && players.apply(playerId, player_action::ChangeLocation{target.zone, 0, target.position}).applied;
    }
    if (player.getZone() == Zone::Town) {
        if (townFarmDoor && farmTown && town.getMap().sameBaseCell(player.getPosition(), *townFarmDoor)) {
            const ReturnPoint target{Zone::Farm, farmTown->returnPosition};
            return canArrive(player, target, farm, town) && players.apply(playerId, player_action::ChangeLocation{target.zone, 0, target.position}).applied;
        }
        if (townMineDoor && town.getMap().sameBaseCell(player.getPosition(), *townMineDoor)) {
            return enterMine(players, playerId, {Zone::Town, *townMineDoor}, dungeon);
        }
    }
    if (entrance && player.getZone() == entrance->zone) {
        const bool atEntrance = entrance->zone == Zone::Farm ? farm.sameBaseCell(player.getPosition(), entrance->position) :
            town.getMap().sameBaseCell(player.getPosition(), entrance->position);
        if (atEntrance) {
            return enterMine(players, playerId, {entrance->zone, entrance->returnPosition}, dungeon);
        }
    }
    return false;
}

void WorldTravel::finishDay() { mineReturns.clear(); }

void WorldTravel::onTeleport(uint16_t playerId, const PlayerSnapshot& origin, const PlayerSnapshot& destination, const FarmManager& farm) {
    if (destination.getZone() != Zone::Dungeon) {
        mineReturns.erase(playerId);
    } else if (origin.getZone() != Zone::Dungeon) {
        if (origin.getZone() == Zone::Farm || origin.getZone() == Zone::Town) {
            mineReturns.insert_or_assign(playerId, ReturnPoint{origin.getZone(), origin.getPosition()});
        } else if (entrance) {
            mineReturns.insert_or_assign(playerId, ReturnPoint{entrance->zone, entrance->returnPosition});
        } else {
            if (const auto outside = farm.spawnFor(origin)) {
                mineReturns.insert_or_assign(playerId, ReturnPoint{Zone::Farm, outside->position});
            }
        }
    }
}
