#include "zone_tab.h"

#include <QHBoxLayout>

#include "side_bar.h"
#include "zone_canvas.h"

namespace {
int leftFor(ElementId element, int x) { return x - ElementCatalog::infoOf(element).tilesWidth / 2; }
int topFor(ElementId element, int y) { return y - ElementCatalog::infoOf(element).tilesHeight + 1; }
}

ZoneTab::ZoneTab(ZoneMap& zone, const Sprites& sprites, const std::vector<TileTextureId>& floors, const std::vector<ElementId>& elements, QWidget* parent):
        QWidget(parent), zone(zone), canvas(new ZoneCanvas(zone, sprites, this)), current{Tool::Kind::Move, {}, {}} {
    SideBar* sideBar = new SideBar(sprites, floors, elements, this);

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->addWidget(sideBar);
    layout->addWidget(canvas, 1);

    connect(sideBar, &SideBar::toolSelected, this, &ZoneTab::selectTool);
    connect(canvas, &ZoneCanvas::tileClicked, this, &ZoneTab::onTileClicked);
    connect(canvas, &ZoneCanvas::tileDragged, this, &ZoneTab::onTileDragged);
    connect(canvas, &ZoneCanvas::tileHovered, this, &ZoneTab::onTileHovered);
    connect(canvas, &ZoneCanvas::rightClicked, this, &ZoneTab::cancelMove);
    connect(canvas, &ZoneCanvas::hoverLeft, canvas, &ZoneCanvas::clearGhost);
}

void ZoneTab::selectTool(const Tool& tool) {
    cancelMove();
    current = tool;
    canvas->clearGhost();
}

void ZoneTab::onTileClicked(int x, int y) {
    switch (current.kind) {
        case Tool::Kind::PaintFloor:
            zone.setFloor(x, y, current.floor);
            canvas->refreshTile(x, y);
            break;

        case Tool::Kind::PlaceElement:
            if (zone.place(current.element, leftFor(current.element, x), topFor(current.element, y))) {
                canvas->refreshElements();
            }
            onTileHovered(x, y);
            break;

        case Tool::Kind::Move:
            if (!moving) {
                moving = zone.elementAt(x, y);
                if (moving) {
                    grabX = x - moving->x;
                    grabY = y - moving->y;
                    zone.removeAt(x, y);
                    canvas->refreshElements();
                    onTileHovered(x, y);
                }
            } else if (zone.place(moving->element, x - grabX, y - grabY)) {
                moving.reset();
                canvas->refreshElements();
                canvas->clearGhost();
            }
            break;

        case Tool::Kind::Erase:
            if (zone.removeAt(x, y)) {
                canvas->refreshElements();
            }
            break;
    }
}

void ZoneTab::onTileDragged(int x, int y) {
    if (current.kind == Tool::Kind::PaintFloor) {
        onTileClicked(x, y);
    }
}

void ZoneTab::onTileHovered(int x, int y) {
    if (current.kind == Tool::Kind::PlaceElement) {
        showGhostAt(current.element, leftFor(current.element, x), topFor(current.element, y));
    } else if (current.kind == Tool::Kind::Move && moving) {
        showGhostAt(moving->element, x - grabX, y - grabY);
    } else {
        canvas->clearGhost();
    }
}

void ZoneTab::showGhostAt(ElementId element, int x, int y) {
    canvas->showGhost(element, x, y, zone.canPlace(element, x, y));
}

void ZoneTab::cancelMove() {
    if (moving) {
        zone.place(moving->element, moving->x, moving->y);
        moving.reset();
        canvas->refreshElements();
        canvas->clearGhost();
    }
}