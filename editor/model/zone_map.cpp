#include "zone_map.h"

namespace {
TileArea footprintAt(ElementId element, int x, int y) {
    const TileArea& footprint = ElementCatalog::infoOf(element).footprint;
    return {x + footprint.x, y + footprint.y, footprint.width, footprint.height};
}

bool overlaps(const TileArea& a, const TileArea& b) {
    return a.x < b.x + b.width && b.x < a.x + a.width && a.y < b.y + b.height && b.y < a.y + a.height;
}

bool covers(const PlacedElement& placed, int tileX, int tileY) {
    const ElementInfo& info = ElementCatalog::infoOf(placed.element);
    return tileX >= placed.x && tileX < placed.x + info.tilesWidth && tileY >= placed.y &&
           tileY < placed.y + info.tilesHeight;
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

    const TileArea footprint = footprintAt(element, x, y);
    if (element.kind != ElementKind::Portal) {
        for (int tileY = footprint.y; tileY < footprint.y + footprint.height; ++tileY) {
            for (int tileX = footprint.x; tileX < footprint.x + footprint.width; ++tileX) {
                if (isLocked(tileX, tileY)) {
                    return false;
                }
            }
        }
    }

    for (const PlacedElement& other: elements) {
        if (overlaps(footprint, footprintAt(other.element, other.x, other.y))) {
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
        if (covers(elements[static_cast<std::size_t>(i)], x, y)) {
            return elements[static_cast<std::size_t>(i)];
        }
    }
    return std::nullopt;
}

bool ZoneMap::removeAt(int x, int y) {
    for (int i = static_cast<int>(elements.size()) - 1; i >= 0; --i) {
        if (covers(elements[static_cast<std::size_t>(i)], x, y)) {
            elements.erase(elements.begin() + i);
            return true;
        }
    }
    return false;
}

int ZoneMap::index(int x, int y) const {
    return y * width + x;
}