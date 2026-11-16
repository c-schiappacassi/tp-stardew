#ifndef ZONE_MAP_H
#define ZONE_MAP_H

#include <optional>
#include <vector>

#include "catalog.h"

struct PlacedElement {
    ElementId element;
    int x;
    int y;
};

class ZoneMap {
public:
    ZoneMap(int width, int height, TileTextureId defaultFloor);

    int getWidth() const;
    int getHeight() const;
    bool inBounds(int x, int y) const;

    TileTextureId getFloor(int x, int y) const;
    void setFloor(int x, int y, TileTextureId floor);

    bool isLocked(int x, int y) const;
    void lock(int x, int y);

    const std::vector<PlacedElement>& getElements() const;
    bool canPlace(ElementId element, int x, int y) const;
    bool place(ElementId element, int x, int y);
    std::optional<PlacedElement> elementAt(int x, int y) const;
    bool removeAt(int x, int y);

private:
    int index(int x, int y) const;

    int width;
    int height;
    std::vector<TileTextureId> floors;
    std::vector<bool> locked;
    std::vector<PlacedElement> elements;
};

#endif  // ZONE_MAP_H