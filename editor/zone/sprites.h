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

    const QPixmap& floor(TileTextureId floor) const;
    const QPixmap& element(ElementId element) const;
    const QPixmap& moveIcon() const;
    const QPixmap& eraseIcon() const;

private:
    const QPixmap& sheet(const std::string& path);

    std::map<std::string, QPixmap> sheets;
    std::map<TileTextureId, QPixmap> floors;
    std::vector<std::pair<ElementId, QPixmap>> elements;
    QPixmap moveCursor;
    QPixmap eraseCross;
};

#endif  // SPRITES_H