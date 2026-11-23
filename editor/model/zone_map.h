#pragma once

#include <map>
#include <optional>
#include <string>
#include <utility>
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

    const std::map<std::string, std::pair<int, int>>& getFixedDoors() const;
    void setFixedDoor(const std::string& key, int x, int y);

private:
    int index(int x, int y) const;
    bool isFixed(const PlacedElement& placed) const;

    int width;
    int height;
    std::vector<TileTextureId> floors;
    std::vector<bool> locked;
    std::vector<PlacedElement> elements;
    std::map<std::string, std::pair<int, int>> fixedDoors;
};