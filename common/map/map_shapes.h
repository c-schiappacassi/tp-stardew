#pragma once
#include <string>
#include <utility>
#include <vector>

#include "common/map/map_data.h"
#include "common/protocol_types.h"

namespace map_rules {

class Shape {
public:
    static Shape parse(const std::vector<std::string>& rows);

    static Shape solidAboveDoor(int width, int height, int doorX, int doorY);

    int width() const { return widthTiles; }
    int height() const { return heightTiles; }
    int doorX() const { return doorTiles.front().first; }
    int doorY() const { return doorTiles.front().second; }
    const std::vector<std::pair<int, int>>& doors() const { return doorTiles; }
    bool blocks(int x, int y) const;
    const std::vector<std::string>& rows() const { return tiles; }

private:
    explicit Shape(std::vector<std::string> rows);

    std::vector<std::string> tiles;
    int widthTiles = 0;
    int heightTiles = 0;
    std::vector<std::pair<int, int>> doorTiles;
};

const Shape& shapeOf(BuildingId id);
const Shape& shapeOf(PortalId id);
int spriteRowsOf(PortalId id);
int doorRowsBelowSprite(PortalId id);
int portalSortY(PortalId id, int spriteBottomY, int tileSize);
int doorRowSortY(BuildingId id, int originY, int tileSize);

TilePosition doorOf(const Shape& shape, int originX, int originY);
TilePosition originOf(const Shape& shape, const TilePosition& door);
std::vector<TilePosition> doorsOf(const Shape& shape, int originX, int originY);

TilePosition returnTileOf(const TilePosition& door);

}