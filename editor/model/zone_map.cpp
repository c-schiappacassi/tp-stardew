#include "zone_map.h"

#include <algorithm>

namespace {
std::vector<TileOffset> occupiedAt(const PlacedElement& placed) {
    std::vector<TileOffset> tiles = ElementCatalog::infoOf(placed.element).occupied;
    for (TileOffset& tile: tiles) {
        tile.x += placed.x;
        tile.y += placed.y;
    }
    return tiles;
}

bool covers(const PlacedElement& placed, int tileX, int tileY) {
    const ElementInfo& info = ElementCatalog::infoOf(placed.element);
    return tileX >= placed.x && tileX < placed.x + info.tilesWidth && tileY >= placed.y &&
           tileY < placed.y + info.tilesHeight;
}

bool boxesOverlap(const PlacedElement& a, const PlacedElement& b) {
    const ElementInfo& first = ElementCatalog::infoOf(a.element);
    const ElementInfo& second = ElementCatalog::infoOf(b.element);
    return a.x < b.x + second.tilesWidth && b.x < a.x + first.tilesWidth && a.y < b.y + second.tilesHeight + 1 &&
           b.y < a.y + first.tilesHeight + 1;
}

bool shareTile(const std::vector<TileOffset>& first, const std::vector<TileOffset>& second) {
    for (const TileOffset& a: first) {
        for (const TileOffset& b: second) {
            if (a.x == b.x && a.y == b.y) {
                return true;
            }
        }
    }
    return false;
}
}

ZoneMap::ZoneMap(int width, int height, TileTextureId defaultFloor):
        width(width),
        height(height),
        floors(static_cast<std::size_t>(width * height), defaultFloor),
        locked(static_cast<std::size_t>(width * height), false) {}

int ZoneMap::getWidth() const {
    return width;
}

int ZoneMap::getHeight() const {
    return height;
}

bool ZoneMap::inBounds(int x, int y) const {
    return x >= 0 && x < width && y >= 0 && y < height;
}

TileTextureId ZoneMap::getFloor(int x, int y) const {
    return floors[static_cast<std::size_t>(index(x, y))];
}

void ZoneMap::setFloor(int x, int y, TileTextureId floor) {
    if (inBounds(x, y) && !isLocked(x, y)) {
        floors[static_cast<std::size_t>(index(x, y))] = floor;
    }
}

bool ZoneMap::isLocked(int x, int y) const {
    return inBounds(x, y) && locked[static_cast<std::size_t>(index(x, y))];
}

void ZoneMap::lock(int x, int y) {
    if (inBounds(x, y)) {
        locked[static_cast<std::size_t>(index(x, y))] = true;
    }
}

const std::vector<PlacedElement>& ZoneMap::getElements() const { return elements; }

bool ZoneMap::canPlace(ElementId element, int x, int y) const {
    const ElementInfo& info = ElementCatalog::infoOf(element);
    if (!inBounds(x, y) || !inBounds(x + info.tilesWidth - 1, y + info.tilesHeight - 1)) {
        return false;
    }

    if (element.kind == ElementKind::Wall) {
        return true;
    }

    const PlacedElement candidate{element, x, y};
    const std::vector<TileOffset> tiles = occupiedAt(candidate);

    if (info.onWall) {
        for (const TileOffset& tile: tiles) {
            if (!isWall(tile.x, tile.y)) {
                return false;
            }
        }
        for (const PlacedElement& other: elements) {
            if (ElementCatalog::infoOf(other.element).onWall && boxesOverlap(candidate, other) &&
                shareTile(tiles, occupiedAt(other))) {
                return false;
            }
        }
        return true;
    }
    std::vector<TileOffset> doorTiles;
    for (const TileOffset& tile: tiles) {
        const bool solid = std::any_of(info.blocked.begin(), info.blocked.end(), [&](const TileOffset& blocked) {
            return blocked.x + x == tile.x && blocked.y + y == tile.y;
        });
        if (!solid) {
            doorTiles.push_back(tile);
        }
    }
    const bool isPortal = element.kind == ElementKind::Portal;
    for (const TileOffset& tile: tiles) {
        if (!inBounds(tile.x, tile.y)) {
            return false;
        }
    }
    for (const TileOffset& tile: isPortal ? doorTiles : tiles) {
        if (isLocked(tile.x, tile.y)) {
            return false;
        }
    }

    for (const PlacedElement& other: elements) {
        if (isPortal && other.element.kind == ElementKind::Wall) {
            continue;
        }
        const bool onlyDoors = isPortal && isFixed(other);
        if (boxesOverlap(candidate, other) && shareTile(onlyDoors ? doorTiles : tiles, occupiedAt(other))) {
            return false;
        }
    }
    return true;
}

bool ZoneMap::place(ElementId element, int x, int y) {
    if (!canPlace(element, x, y)) {
        return false;
    }
    elements.push_back({element, x, y});
    return true;
}

std::optional<PlacedElement> ZoneMap::elementAt(int x, int y) const {
    for (int i = static_cast<int>(elements.size()) - 1; i >= 0; --i) {
        const PlacedElement& placed = elements[static_cast<std::size_t>(i)];
        if (!isFixed(placed) && covers(placed, x, y)) {
            return elements[static_cast<std::size_t>(i)];
        }
    }
    return std::nullopt;
}

bool ZoneMap::removeAt(int x, int y) {
    for (int i = static_cast<int>(elements.size()) - 1; i >= 0; --i) {
        const PlacedElement& placed = elements[static_cast<std::size_t>(i)];
        if (!isFixed(placed) && covers(placed, x, y)) {
            elements.erase(elements.begin() + i);
            return true;
        }
    }
    return false;
}

const std::map<std::string, std::pair<int, int>>& ZoneMap::getFixedDoors() const { return fixedDoors; }

void ZoneMap::setFixedDoor(const std::string& key, int x, int y) { fixedDoors[key] = {x, y}; }

bool ZoneMap::isFixed(const PlacedElement& placed) const {
    if (placed.element.kind == ElementKind::Wall) {
        return true;
    }
    if (placed.element.kind == ElementKind::Portal || ElementCatalog::infoOf(placed.element).onWall) {
        return false;
    }
    for (const TileOffset& tile: occupiedAt(placed)) {
        if (isLocked(tile.x, tile.y)) {
            return true;
        }
    }
    return false;
}

int ZoneMap::index(int x, int y) const {
    return y * width + x;
}

bool ZoneMap::isWall(int x, int y) const {
    if (!isLocked(x, y)) {
        return false;
    }
    if (getFloor(x, y) != TileTextureId::DungeonVoid) {
        return true;
    }
    for (const PlacedElement& placed: elements) {
        if (placed.element.kind == ElementKind::Wall && covers(placed, x, y)) {
            return true;
        }
    }
    return false;
}
