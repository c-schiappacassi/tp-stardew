#include "common/map/map_shapes.h"

#include <map>
#include <stdexcept>
#include <utility>

namespace map_rules {

namespace {

const std::map<BuildingId, Shape>& buildings() {
    static const std::map<BuildingId, Shape> table{
        // Farm house (9x10 tiles)
        {BuildingId::FarmHouse, Shape::parse({
            ".........",
            ".........",
            ".........",
            ".........",
            "#########",
            "#########",
            "#########",
            "#########",
            "#....D..#",
            "####...##"})},
        // Stone cabin (5x7 tiles)
        {BuildingId::StoneCabin, Shape::parse({
            ".....",
            ".....",
            "#####",
            "#####",
            "#####",
            "#####",
            "..D.#"})},
        // Wood cabin (5x7 tiles)
        {BuildingId::WoodCabin, Shape::parse({
            ".....",
            ".....",
            "#####",
            "#####",
            "#####",
            "#####",
            "..D.#"})},
        // Brick cabin (5x7 tiles)
        {BuildingId::BrickCabin, Shape::parse({
            ".....",
            ".....",
            "#####",
            "#####",
            "#####",
            "#####",
            "..D.#"})},
        // Marnie's ranch (8x7 tiles)
        {BuildingId::MarniesShop, Shape::parse({
            "........",
            "........",
            "########",
            "########",
            "########",
            "########",
            "...D.###"})},
        // Pierre's general store (11x10 tiles + 1 fila):
        {BuildingId::PierresShop, Shape::parse({
            "...........",
            "...........",
            "...........",
            "##########.",
            "##########.",
            "##########.",
            "##########.",
            "##########.",
            "##########.",
            "##########.",
            ".....DDD..."})},
        // Robin's carpenter shop (10x13 tiles):
        {BuildingId::RobinsShop, Shape::parse({
            "..........",
            "..........",
            "######....",
            "######....",
            "######....",
            "##########",
            "##########",
            "#..#######",
            "#..#######",
            "#..#######",
            "#..#######",
            "##########",
            ".....DDD.."})},
        // Clint's blacksmith (9x8 tiles + 1 fila):
        {BuildingId::ClintsShop, Shape::parse({
            ".........",
            ".........",
            ".........",
            "#####....",
            "########.",
            "########.",
            "########.",
            "########.",
            "...DD...."})},
        // Community center (12x10 tiles):
        {BuildingId::ComunityCenter, Shape::parse({
            "............",
            "............",
            "............",
            "............",
            "############",
            "############",
            "############",
            "############",
            "############",
            "#####DD#####"})},
    };
    return table;
}

const std::map<PortalId, Shape>& portals() {
    static const std::map<PortalId, Shape> table{
        {PortalId::FarmToMine,  Shape::parse({"...","...","...",".D."})},
        {PortalId::ToNextLevel, Shape::parse({"D"})},
        {PortalId::MineToFarm, Shape::parse({".",".",".",".","D","D"})},
    };
    return table;
}

} 

Shape::Shape(std::vector<std::string> rows): tiles(std::move(rows)) {
    if (tiles.empty() || tiles.front().empty()) {
        throw std::invalid_argument("Shape: empty");
    }
    heightTiles = static_cast<int>(tiles.size());
    widthTiles = static_cast<int>(tiles.front().size());

    for (int y = 0; y < heightTiles; ++y) {
        const std::string& row = tiles[static_cast<std::size_t>(y)];
        if (static_cast<int>(row.size()) != widthTiles) {
            throw std::invalid_argument("Shape: rows must all have the same length");
        }
        for (int x = 0; x < widthTiles; ++x) {
            const char tile = row[static_cast<std::size_t>(x)];
            if (tile == 'D') {
                doorTiles.emplace_back(x, y);
            } else if (tile != '#' && tile != '.') {
                throw std::invalid_argument(std::string("Shape: unknown tile '") + tile + "'");
            }
        }
    }
    if (doorTiles.empty()) {
        throw std::invalid_argument("Shape: needs at least one door 'D'");
    }
}

Shape Shape::parse(const std::vector<std::string>& rows) { return Shape(rows); }

Shape Shape::solidAboveDoor(int width, int height, int doorX, int doorY) {
    if (width <= 0 || height <= 0 || doorX < 0 || doorX >= width || doorY < 0 || doorY >= height) {
        throw std::invalid_argument("Shape: door outside the shape");
    }
    std::vector<std::string> rows(static_cast<std::size_t>(height), std::string(static_cast<std::size_t>(width), '.'));
    for (int y = 0; y < doorY; ++y) {
        rows[static_cast<std::size_t>(y)] = std::string(static_cast<std::size_t>(width), '#');
    }
    rows[static_cast<std::size_t>(doorY)][static_cast<std::size_t>(doorX)] = 'D';
    return Shape(std::move(rows));
}

bool Shape::blocks(int x, int y) const {
    return x >= 0 && x < widthTiles && y >= 0 && y < heightTiles &&
           tiles[static_cast<std::size_t>(y)][static_cast<std::size_t>(x)] == '#';
}

const Shape& shapeOf(BuildingId id) {
    const auto found = buildings().find(id);
    if (found == buildings().end()) {
        throw std::out_of_range("building without a shape");
    }
    return found->second;
}

int doorRowSortY(BuildingId id, int originY, int tileSize) {
    return originY + shapeOf(id).doorY() * tileSize;
}

int spriteRowsOf(PortalId id) {
    switch (id) {
        case PortalId::FarmToMine: return 3;
        case PortalId::ToNextLevel: return 1;
        case PortalId::MineToFarm: return 4;
    }
    throw std::out_of_range("portal without a sprite");
}

int doorRowsBelowSprite(PortalId id) {
    return shapeOf(id).doorY() - (spriteRowsOf(id) - 1);
}

int portalSortY(PortalId id, int spriteBottomY, int tileSize) {
    return spriteBottomY - (spriteRowsOf(id) - shapeOf(id).doorY()) * tileSize;
}

const Shape& shapeOf(PortalId id) {
    const auto found = portals().find(id);
    if (found == portals().end()) {
        throw std::out_of_range("portal without a shape");
    }
    return found->second;
}

TilePosition doorOf(const Shape& shape, int originX, int originY) {
    return {originX + shape.doorX(), originY + shape.doorY()};
}

TilePosition originOf(const Shape& shape, const TilePosition& door) {
    return {door.x - shape.doorX(), door.y - shape.doorY()};
}

std::vector<TilePosition> doorsOf(const Shape& shape, int originX, int originY) {
    std::vector<TilePosition> doors;
    for (const auto& [x, y]: shape.doors()) {
        doors.push_back({originX + x, originY + y});
    }
    return doors;
}

TilePosition returnTileOf(const TilePosition& door) { return {door.x, door.y + 1}; }

}