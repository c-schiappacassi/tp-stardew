#include "sprites.h"

#include <stdexcept>

#include "editor_constants.h"

namespace {
const int TILE_PIXELS = 16;
}

Sprites::Sprites() {
    for (const FloorInfo& info: FloorCatalog::all()) {
        floors[info.floor] = sheet(info.sheet).copy(info.spriteX, info.spriteY, TILE_PIXELS, TILE_PIXELS);
    }
    for (const ElementInfo& info: ElementCatalog::all()) {
        const QPixmap sprite = sheet(info.sheet).copy(info.spriteX, info.spriteY, info.spriteWidth, info.spriteHeight);
        elements.emplace_back(info.element, sprite);
    }
    moveCursor = sheet("menus/cursors.png").copy(0, 0, TILE_PIXELS, TILE_PIXELS);
    eraseCross = sheet("menus/cursors.png").copy(268, 470, TILE_PIXELS, TILE_PIXELS);
}

const QPixmap& Sprites::floor(TileTextureId floor) const { return floors.at(floor); }

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