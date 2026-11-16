#ifndef ZONE_TAB_H
#define ZONE_TAB_H

#include <vector>
#include <optional>

#include <QWidget>

#include "../model/catalog.h"
#include "sprites.h"
#include "../model/zone_map.h"
#include "tool.h"

class ZoneCanvas;

class ZoneTab : public QWidget {
    Q_OBJECT

public:
    ZoneTab(ZoneMap& zone, const Sprites& sprites, const std::vector<TileTextureId>& floors, const std::vector<ElementId>& elements, QWidget* parent = nullptr);

private:
    void selectTool(const Tool& tool);
    void onTileClicked(int x, int y);
    void onTileDragged(int x, int y);
    void onTileHovered(int x, int y);
    void showGhostAt(ElementId element, int x, int y);
    void cancelMove();

    ZoneMap& zone;
    ZoneCanvas* canvas;
    Tool current;
    std::optional<PlacedElement> moving;
    int grabX = 0;
    int grabY = 0;
};

#endif  // ZONE_TAB_H