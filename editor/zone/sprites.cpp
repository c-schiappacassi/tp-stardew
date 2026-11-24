#include "sprites.h"

#include <stdexcept>

#include <QPainter>

#include "editor_constants.h"

namespace {
const int TILE_PIXELS = 16;
}

Sprites::Sprites() {
    const FloorInfo& dungeonFloor = FloorCatalog::infoOf(TileTextureId::DungeonFloor);
    const QPixmap under = sheet(dungeonFloor.sheet).copy(dungeonFloor.spriteX, dungeonFloor.spriteY, TILE_PIXELS, TILE_PIXELS);
    for (const FloorInfo& info: FloorCatalog::all()) {
        const QPixmap whole = sheet(info.sheet).copy(info.spriteX, info.spriteY, info.spriteWidth, info.spriteHeight);
        floors[info.floor] = whole;
        //se corta en pedazos de 16x16, fila por fila
        std::vector<QPixmap>& tiles = floorTiles[info.floor];
        for (int y = 0; y < info.spriteHeight; y += TILE_PIXELS) {
            for (int x = 0; x < info.spriteWidth; x += TILE_PIXELS) {
                QPixmap tile(TILE_PIXELS, TILE_PIXELS);
                tile.fill(Qt::transparent);
                QPainter painter(&tile);
                if (info.overDungeonFloor) {
                    painter.drawPixmap(0, 0, under);
                }
                painter.drawPixmap(0, 0, whole, x, y, TILE_PIXELS, TILE_PIXELS);
                painter.end();
                tiles.push_back(tile);
            }
        }
    }
    for (const ElementInfo& info: ElementCatalog::all()) {
        const QPixmap sprite = sheet(info.sheet).copy(info.spriteX, info.spriteY, info.spriteWidth, info.spriteHeight);
        elements.emplace_back(info.element, sprite);
    }
    moveCursor = sheet("menus/cursors.png").copy(0, 0, TILE_PIXELS, TILE_PIXELS);
    eraseCross = sheet("menus/cursors.png").copy(268, 470, TILE_PIXELS, TILE_PIXELS);
}

const QPixmap& Sprites::floor(TileTextureId floor) const { return floors.at(floor); }

const QPixmap& Sprites::floorTile(TileTextureId floor, int x, int y) const {
    const FloorInfo& info = FloorCatalog::infoOf(floor);
    const int columns = info.spriteWidth / TILE_PIXELS;
    const int rows = info.spriteHeight / TILE_PIXELS;
    return floorTiles.at(floor)[static_cast<std::size_t>((y % rows) * columns + x % columns)];
}

const QPixmap& Sprites::element(ElementId element) const {
    for (const auto& [id, sprite]: elements) {
        if (id == element) {
            return sprite;
        }
    }
    throw std::out_of_range("elemento sin sprite");
}

const QPixmap& Sprites::moveIcon() const {
    return moveCursor;
}

const QPixmap& Sprites::eraseIcon() const {
    return eraseCross;
}

const QPixmap& Sprites::sheet(const std::string& path) {
    auto found = sheets.find(path);
    if (found == sheets.end()) {
        found = sheets.emplace(path, QPixmap(SPRITES_PATH + QString::fromStdString(path))).first;
    }
    return found->second;
}