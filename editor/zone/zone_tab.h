#ifndef ZONE_TAB_H
#define ZONE_TAB_H

#include <vector>

#include <QWidget>

#include "../model/catalog.h"
#include "sprites.h"

class ZoneTab : public QWidget {
    Q_OBJECT

public:
    ZoneTab(const Sprites& sprites, const std::vector<TileTextureId>& floors, const std::vector<ElementId>& elements, QWidget* parent = nullptr);
};

#endif  // ZONE_TAB_H