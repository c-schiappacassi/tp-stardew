#include "world_travel.h"

#include "dungeon/dungeon_entrance.h"
#include "dungeon/dungeon_manager.h"
#include "farm/farm_manager.h"
#include "farm/loading/farm_layout.h"
#include "player/player.h"
#include "town/town_map.h"
#include "interiors/interior_manager.h"

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

bool WorldTravel::configureEntrance(const DungeonEntrance& data, const FarmManager& farm,const TownMap& town) {
    const CollisionBox playerBox{entity_dimensions::PLAYER};
    if ((data.zone != Zone::Farm && data.zone != Zone::Town) ||
        !data.terrain.isWalkable(playerBox.at(data.position)) ||
        !data.terrain.isWalkable(playerBox.at(data.returnPosition))) return false;
    if (data.zone == Zone::Farm &&
        (!farm.isInitialized() || !farm.canWalk(playerBox.at(data.position)) ||
         !farm.canWalk(playerBox.at(data.returnPosition)))) return false;
    if (data.zone == Zone::Town &&
        (!town.canWalk(playerBox.at(data.position)) ||
         !town.canWalk(playerBox.at(data.returnPosition)))) return false;
    entrance = Entrance{data.zone, data.position, data.returnPosition};
    return true;
}

bool WorldTravel::canConnectTown(const TownMap& town, const FarmManager& farm) const {
    if (!farm.isInitialized() || !farmTown || !town.connectionTo(Zone::Farm)) {
        return false;
    }
    const CollisionBox playerBox{entity_dimensions::PLAYER};
    return farm.canWalk(playerBox.at(farmTown->door)) &&
           farm.canWalk(playerBox.at(farmTown->returnPosition));
}

std::optional<Position> WorldTravel::getDungeonEntrance(Zone zone) const {
    if (!entrance || entrance->zone != zone) {
        return std::nullopt;
    }
    return entrance->position;
}

bool WorldTravel::canArrive(const Player& player, const ReturnPoint& target,const FarmManager& farm, const TownMap& town) const {
    const auto bounds = player.getCollisionBounds(target.position);
    if (target.zone == Zone::Farm) return farm.isInitialized() && farm.canWalk(bounds);
    return target.zone == Zone::Town && town.canWalk(bounds);
}

bool WorldTravel::enterMine(Player& player, uint16_t playerId, const ReturnPoint& origin,DungeonManager& dungeon) {
    if (!dungeon.enter(player)) return false;
    mineReturns.insert_or_assign(playerId, origin);
    return true;
}

bool WorldTravel::use(Player& player, uint16_t playerId, DungeonManager& dungeon,const FarmManager& farm, const TownMap& town, const InteriorManager& interiors) {
    if (!player.canAct()) return false;
    const auto terrainFor = [&](const WorldLocation& location) -> const ZoneMap* {
        if (location.zone == Zone::Farm) return &farm.getMap();
        if (location.zone == Zone::Town) return &town.getMap();
        if (location.zone == Zone::InsideHouse && location.interior) {
            const auto* room = interiors.find(*location.interior);
            return room ? &room->terrain : nullptr;
        }
        return nullptr;
    };
    for (const auto& portal: portals) {
        const auto& source = portal.source;
        const auto* terrain = terrainFor(source);
        if (source.zone != player.getZone() || source.level != player.getDungeonLevel() ||
            source.interior != player.getInterior() || !terrain ||
            !terrain->sameBaseCell(player.getPosition(), source.position)) continue;
        const auto& target = portal.destination;
        const auto* destination = terrainFor(target);
        const auto bounds = player.getCollisionBounds(target.position);
        const bool canWalk = target.zone == Zone::InsideHouse && target.interior
            ? interiors.canWalk(*target.interior, bounds)
            : canArrive(player, {target.zone, target.position}, farm, town);
        return destination && canWalk &&
               player.changeLocation(target.zone, target.level, target.position, target.interior);
    }
    const auto townFarmDoor = town.connectionTo(Zone::Farm);
    const auto townMineDoor = town.connectionTo(Zone::Dungeon);
    if (player.getZone() == Zone::Dungeon) {
        if (!dungeon.canExit(player)) return dungeon.advanceRoom(player);
        const auto found = mineReturns.find(playerId);
        if (found == mineReturns.end() && !entrance) return false;
        const ReturnPoint target = found != mineReturns.end()
            ? found->second : ReturnPoint{entrance->zone, entrance->returnPosition};
        if (!canArrive(player, target, farm, town) ||
            !player.changeLocation(target.zone, 0, target.position)) return false;
        mineReturns.erase(playerId);
        return true;
    }
    if (player.getZone() == Zone::Farm && farmTown && townFarmDoor &&
        farm.getMap().sameBaseCell(player.getPosition(), farmTown->door)) {
        const ReturnPoint target{Zone::Town, *townFarmDoor};
        return canArrive(player, target, farm, town) &&
               player.changeLocation(target.zone, 0, target.position);
    }
    if (player.getZone() == Zone::Town) {
        if (townFarmDoor && farmTown && town.getMap().sameBaseCell(player.getPosition(), *townFarmDoor)) {
            const ReturnPoint target{Zone::Farm, farmTown->returnPosition};
            return canArrive(player, target, farm, town) &&
                   player.changeLocation(target.zone, 0, target.position);
        }
        if (townMineDoor && town.getMap().sameBaseCell(player.getPosition(), *townMineDoor)) {
            return enterMine(player, playerId, {Zone::Town, *townMineDoor}, dungeon);
        }
    }
    if (entrance && player.getZone() == entrance->zone) {
        const auto& terrain = entrance->zone == Zone::Farm ? farm.getMap() : town.getMap();
        if (terrain.sameBaseCell(player.getPosition(), entrance->position)) {
            return enterMine(player, playerId, {entrance->zone, entrance->returnPosition}, dungeon);
        }
    }
    return false;
}

void WorldTravel::finishDay() { mineReturns.clear(); }

void WorldTravel::onTeleport(uint16_t playerId, const Player& origin, const Player& destination,const FarmManager& farm) {
    if (destination.getZone() != Zone::Dungeon) {
        mineReturns.erase(playerId);
    } else if (origin.getZone() != Zone::Dungeon) {
        if (origin.getZone() == Zone::Farm || origin.getZone() == Zone::Town) {
            mineReturns.insert_or_assign(playerId, ReturnPoint{origin.getZone(), origin.getPosition()});
        } else if (entrance) {
            mineReturns.insert_or_assign(playerId, ReturnPoint{entrance->zone, entrance->returnPosition});
        } else {
            Player outside = origin;
            if (farm.spawnPlayer(outside)) {
                mineReturns.insert_or_assign(playerId, ReturnPoint{Zone::Farm, outside.getPosition()});
            }
        }
    }
}
