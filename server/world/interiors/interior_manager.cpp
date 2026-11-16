#include "interior_manager.h"

#include <algorithm>
#include <cstdlib>
#include <limits>
#include <set>

#include "entities/collision_box.h"
#include "player/player.h"

namespace {
const CollisionBox BED_BOX{22, 32};

bool validPosition(const Position& pos) {
    return pos.getX() >= 0 && pos.getY() >= 0 &&
           pos.getX() <= std::numeric_limits<uint16_t>::max() &&
           pos.getY() <= std::numeric_limits<uint16_t>::max();
}

bool canApproachBed(const InteriorLayout& room, const Position& pos, const Position& bed) {
    const int dx = bed.getX() - pos.getX();
    const int dy = bed.getY() - pos.getY();
    const int steps = std::max(std::abs(dx), std::abs(dy));
    if (steps > 22) return false;
    const CollisionBox playerBox{entity_dimensions::PLAYER};
    for (int step = 0; step <= steps; ++step) {
        const Position point{pos.getX() + dx * step / std::max(1, steps),pos.getY() + dy * step / std::max(1, steps)};
        if (!room.terrain.isWalkable(playerBox.at(point))) return false;
    }
    return true;
}

ZoneMap walkingMap(const InteriorLayout& room) {
    const CollisionBox playerBox{entity_dimensions::PLAYER};
    const auto positions = room.terrain.walkableBasePositions();
    int rows = 0;
    int columns = 0;
    for (const auto& pos: positions) {
        const auto cell = room.terrain.baseCellAt(pos).value();
        rows = std::max(rows, cell.first + 1);
        columns = std::max(columns, cell.second + 1);
    }
    std::vector<std::vector<bool>> floor(rows, std::vector<bool>(columns, false));
    for (const auto& pos: positions) {
        const auto cell = room.terrain.baseCellAt(pos).value();
        floor[cell.first][cell.second] = room.terrain.isWalkable(playerBox.at(pos)) &&
            std::none_of(room.beds.begin(), room.beds.end(), [&](const auto& bed) {
                return BED_BOX.at(bed).intersects(playerBox.at(pos));
            });
    }
    return ZoneMap{floor, room.terrain.getCellSize()};
}
}

bool InteriorManager::initialize(const std::vector<InteriorLayout>& layouts) {
    std::set<std::pair<InteriorId, uint8_t>> registered;
    const CollisionBox playerBox{entity_dimensions::PLAYER};
    for (const auto& room: layouts) {
        if (room.location.interior_id > InteriorId::ComunityCenter ||
            (room.location.interior_id == InteriorId::PlayerHouse && room.location.house_index >= 4) ||
            (room.location.interior_id != InteriorId::PlayerHouse && room.location.house_index != 0) ||
            !registered.emplace(room.location.interior_id, room.location.house_index).second ||
            (room.outside.zone != Zone::Farm && room.outside.zone != Zone::Town) ||
            room.outside.level != 0 || room.outside.interior ||
            !validPosition(room.outside.position) || !validPosition(room.returnPosition) ||
            !validPosition(room.entry) || !validPosition(room.exit) ||
            !room.terrain.isWalkable(playerBox.at(room.entry)) ||
            !room.terrain.isWalkable(playerBox.at(room.exit)) ||
            !room.terrain.hasBasePath(room.entry, room.exit)) return false;
        std::vector<CollisionBounds> beds;
        for (const auto& bed: room.beds) {
            const auto bounds = BED_BOX.at(bed);
            if (!validPosition(bed) || !room.terrain.isWalkable(bounds) ||
                bounds.intersects(playerBox.at(room.entry)) ||
                bounds.intersects(playerBox.at(room.exit)) ||
                std::any_of(beds.begin(), beds.end(), [&](const auto& other) {
                    return bounds.intersects(other);
                })) return false;
            beds.push_back(bounds);
        }
    }
    for (const auto& room: layouts) {
        const auto walking = walkingMap(room);
        if (!walking.hasBasePath(room.entry, room.exit)) return false;
        const auto positions = walking.walkableBasePositions();
        for (const auto& bed: room.beds) {
            if (std::none_of(positions.begin(), positions.end(), [&](const auto& pos) {
                return walking.hasBasePath(room.entry, pos) && canApproachBed(room, pos, bed);
            })) {
                return false;
            }
        }
    }
    rooms = layouts;
    return true;
}

const InteriorLayout* InteriorManager::find(const InteriorLocation& location) const {
    const auto it = std::find_if(rooms.begin(), rooms.end(), [&](const auto& room) {
        return room.location == location;
    });
    return it == rooms.end() ? nullptr : &*it;
}

bool InteriorManager::canWalk(const InteriorLocation& location, const CollisionBounds& bounds) const {
    const auto* room = find(location);
    return room && room->terrain.isWalkable(bounds) &&
           std::none_of(room->beds.begin(), room->beds.end(), [&](const auto& bed) {
               return BED_BOX.at(bed).intersects(bounds);
           });
}

bool InteriorManager::useBed(Player& player) const {
    if (player.getZone() != Zone::InsideHouse || !player.getInterior()) {
        player.leaveBedArea();
        return false;
    }
    const auto* room = find(*player.getInterior());
    if (!room) return false;
    const auto& pos = player.getPosition();
    for (const auto& bed: room->beds) {
        if (canApproachBed(*room, pos, bed)) return player.canUseBed() && player.sleep();
    }
    player.leaveBedArea();
    return false;
}

bool InteriorManager::recoverPlayer(Player& player, uint16_t playerId) const {
    const InteriorLayout* house = playerId >= 1 && playerId <= 4
        ? find({InteriorId::PlayerHouse, static_cast<uint8_t>(playerId - 1)}) : nullptr;
    if (!house || house->beds.empty()) {
        const auto fallback = std::find_if(rooms.begin(), rooms.end(), [](const auto& room) {
            return room.location.interior_id == InteriorId::PlayerHouse && !room.beds.empty();
        });
        if (fallback == rooms.end()) return false;
        house = &*fallback;
    }
    const auto walking = walkingMap(*house);
    for (const auto& bed: house->beds) {
        const Position foot{bed.getX(), bed.getY() + house->terrain.getCellSize()};
        const auto canRecover = [&](const Position& pos) {
            return canWalk(house->location, player.getCollisionBounds(pos)) &&
                   walking.hasBasePath(house->entry, pos) && canApproachBed(*house, pos, bed);
        };
        if (canRecover(foot)) {
            return player.restoreLocation(Zone::InsideHouse, 0, foot, house->location);
        }
        for (const auto& pos: walking.walkableBasePositions()) {
            if (canRecover(pos)) {
                return player.restoreLocation(Zone::InsideHouse, 0, pos, house->location);
            }
        }
    }
    return false;
}

InteriorData InteriorManager::dataFor(const InteriorLocation& location) const {
    return InteriorData{location.interior_id, location.house_index, {}};
}

void InteriorManager::completeMap(ConstMapData& map) const {
    for (const auto& room: rooms) {
        BackgroundData background = room.background;
        background.portals.push_back({room.exit, PortalId::ToNextLevel});
        for (const auto& bed: room.beds) {
            background.furniture.push_back({bed, FurnitureId::Bed, Direction::None});
            background.furniture.push_back({bed, FurnitureId::BedSheet, Direction::None});
        }
        switch (room.location.interior_id) {
            case InteriorId::PlayerHouse:
                if (map.farm_houses.size() <= room.location.house_index) {
                    map.farm_houses.resize(room.location.house_index + 1);
                }
                map.farm_houses[room.location.house_index] = std::move(background);
                break;
            case InteriorId::PierresShop: map.pierres = std::move(background); break;
            case InteriorId::RobinsShop: map.robins = std::move(background); break;
            case InteriorId::ClintsShop: map.clints = std::move(background); break;
            case InteriorId::MarniesShop: map.marnies = std::move(background); break;
            case InteriorId::ComunityCenter: map.community_center = std::move(background); break;
        }
        auto& outside = room.outside.zone == Zone::Farm ? map.farm : map.town;
        outside.portals.push_back({room.outside.position, PortalId::ToNextLevel});
    }
}
