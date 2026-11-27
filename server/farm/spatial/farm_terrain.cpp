#include "farm_terrain.h"

#include "server_constants.h"

#include <limits>
#include <stdexcept>

#include "player/queries/player_snapshot.h"

bool FarmTerrain::validBase(const Position& base) {
    return base.getX() >= 0 && base.getY() > 0 && base.getX() <= std::numeric_limits<uint16_t>::max() && base.getY() <= std::numeric_limits<uint16_t>::max();
}

bool FarmTerrain::initialize(const FarmLayout& layout) {
    if (!validBase(layout.spawn))
        return false;
    for (const auto& base: layout.terrain.walkableBasePositions()) {
        if (!validBase(base))
            return false;
    }
    auto reserved = layout.reservedPositions;
    reserved.push_back(layout.spawn);
    std::vector<Position> triggers;
    if (layout.mineDoor) triggers.push_back(*layout.mineDoor);
    if (layout.townDoor) triggers.push_back(*layout.townDoor);
    for (const auto& room: layout.interiors) {
        if (room.outside.zone == Zone::Farm) {
            triggers.push_back(room.outside.position);
            reserved.push_back(room.returnPosition);
        }
    }
    for (std::size_t i = 0; i < triggers.size(); ++i) {
        if (!validBase(triggers[i]) || !layout.terrain.isWalkable(CollisionBox{entity_dimensions::PLAYER}.at(triggers[i])) ||
            !layout.terrain.hasBasePath(layout.spawn, triggers[i]))
            return false;
        for (std::size_t j = 0; j < i; ++j) {
            if (layout.terrain.sameBaseCell(triggers[i], triggers[j]))
                return false;
        }
        reserved.push_back(triggers[i]);
    }
    if (layout.playerHouses == 0 || layout.playerHouses > 4 || (layout.mineReturn && !layout.mineDoor) || (layout.townReturn && !layout.townDoor))
        return false;
    if (layout.mineReturn) reserved.push_back(*layout.mineReturn);
    if (layout.townReturn) reserved.push_back(*layout.townReturn);
    if (!space.setMap(layout.terrain) || !space.setReservedPositions(reserved))
        return false;
    background = layout.background;
    dirtCells.clear();
    for (const auto& tile: background) {
        if (tile.textureId == static_cast<uint16_t>(TileTextureId::Dirt))
            dirtCells.emplace(tile.tile.getY(), tile.tile.getX());
    }
    spawnPosition = layout.spawn;
    initialized = true;
    return true;
}

bool FarmTerrain::isInitialized() const { return initialized; }
bool FarmTerrain::setMap(const ZoneMap& map) { return space.setMap(map); }
bool FarmTerrain::setReservedPositions(const std::vector<Position>& positions) { return space.setReservedPositions(positions); }
void FarmTerrain::setObstacles(const std::vector<CollisionBounds>& bounds) { space.setObstacles(bounds); }
bool FarmTerrain::canWalk(const CollisionBounds& bounds) const { return space.canWalk(bounds); }
bool FarmTerrain::sameBaseCell(const Position& first, const Position& second) const { return getMap().sameBaseCell(first, second); }

bool FarmTerrain::isCultivable(const Position& position) const {
    const auto cell = space.cellAt(position);
    return cell && dirtCells.contains(*cell);
}

std::optional<WorldLocation> FarmTerrain::spawnFor(const PlayerSnapshot& player) const {
    if (!initialized)
        return std::nullopt;
    if (canWalk(player.getCollisionBounds(spawnPosition)))
        return WorldLocation{Zone::Farm, 0, std::nullopt, spawnPosition};
    for (const auto& position: getMap().walkableBasePositions()) {
        if (canWalk(player.getCollisionBounds(position)))
            return WorldLocation{Zone::Farm, 0, std::nullopt, position};
    }
    return std::nullopt;
}

std::vector<TileData> FarmTerrain::backgroundData() const {
    std::vector<TileData> result;
    result.reserve(background.size());
    for (const auto& tile: background) {
        if (tile.textureId > static_cast<uint16_t>(TileTextureId::Dirt))
            throw std::runtime_error(server_constants::UNKNOWN_FARM_TEXTURE_ID);
        result.push_back({Position{tile.tile.getX() * 16, tile.tile.getY() * 16}, static_cast<TileTextureId>(tile.textureId)});
    }
    return result;
}

PlacementSpace& FarmTerrain::getSpace() { return space; }
const PlacementSpace& FarmTerrain::getSpace() const { return space; }
const ZoneMap& FarmTerrain::getMap() const { return space.getMap(); }
