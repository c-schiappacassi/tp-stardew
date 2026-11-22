#ifndef SPRITES_H
#define SPRITES_H

#include <map>
#include <string>
#include <utility>
#include <vector>

#include <QPixmap>

#include "../model/catalog.h"

class Sprites {
public:
    Sprites();

    const QPixmap& floor(TileTextureId floor) const;               // el recorte entero (icono)
    const QPixmap& floorTile(TileTextureId floor, int x, int y) const;  // lo que va en el tile (x, y)
    const QPixmap& element(ElementId element) const;
    const QPixmap& moveIcon() const;
    const QPixmap& eraseIcon() const;

private:
    const QPixmap& sheet(const std::string& path);

    std::map<std::string, QPixmap> sheets;
    std::map<TileTextureId, QPixmap> floors;
    std::map<TileTextureId, std::vector<QPixmap>> floorTiles;  // un pedazo de 16x16 por tile del patron
    std::vector<std::pair<ElementId, QPixmap>> elements;
    QPixmap moveCursor;
    QPixmap eraseCross;
};

#endif  // SPRITES_H