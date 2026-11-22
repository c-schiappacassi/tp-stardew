#ifndef CATALOG_H
#define CATALOG_H

#include <string>
#include <vector>

#include "protocol_types.h"

struct FloorInfo {
    TileTextureId floor;
    std::string name;
    std::string sheet;
    int spriteX;
    int spriteY;
    int spriteWidth = 16;
    int spriteHeight = 16;
    bool walkable = true;
    bool overDungeonFloor = false;
};

class FloorCatalog {
public:
    static const FloorInfo& infoOf(TileTextureId floor);
    static const std::vector<FloorInfo>& all();
};


enum class ElementKind { Building, Decor, Portal, Wall };

struct ElementId {
    ElementKind kind;
    int id;

    bool operator==(const ElementId& other) const {
        return kind == other.kind && id == other.id;
    }
};

ElementId buildingElement(BuildingId id);
ElementId decorElement(DecorId id);
ElementId portalElement(PortalId id);
ElementId wallElement(FramePieceId id);

struct TileArea {
    int x;
    int y;
    int width;
    int height;
};

struct ElementInfo {
    ElementId element;
    std::string name;
    std::string sheet;
    int spriteX;
    int spriteY;
    int spriteWidth;
    int spriteHeight;
    int tilesWidth;
    int tilesHeight;
    TileArea footprint;  // que tiles ocupa en el PISO
    int doorColumn = 0;
};

class ElementCatalog {
public:
    static const ElementInfo& infoOf(ElementId element);
    static const std::vector<ElementInfo>& all();
};

#endif  // ELEMENT_CATALOG_H