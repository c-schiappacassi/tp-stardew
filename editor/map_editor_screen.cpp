#include "map_editor_screen.h"

#include <QPixmap>
#include <QTabWidget>
#include <QVBoxLayout>

#include "editor_constants.h"
#include "sprite_button.h"
#include "zone/zone_tab.h"

namespace {
const QRect backNormalRect(296, 252, 66, 28);
const QRect backHoverRect(296, 280, 66, 26);
const QSize backSize(160, 68);

std::vector<TileTextureId> allFloors() {
    std::vector<TileTextureId> floors;
    for (const FloorInfo& info: FloorCatalog::all()) {
        floors.push_back(info.floor);
    }
    return floors;
}

std::vector<ElementId> allElements() {
    std::vector<ElementId> elements;
    for (const ElementInfo& info: ElementCatalog::all()) {
        elements.push_back(info.element);
    }
    return elements;
}
}

MapEditorScreen::MapEditorScreen(QWidget* parent): QWidget(parent) {
    const QPixmap titleSheet(MENUS_PATH + "title_screen.png");
    SpriteButton* backButton = new SpriteButton(titleSheet.copy(backNormalRect), titleSheet.copy(backHoverRect), this);
    backButton->setFixedSize(backSize);

    QTabWidget* tabs = new QTabWidget(this);
    tabs->addTab(new ZoneTab(map.farm, sprites, allFloors(), allElements(), this), "Farm");
    tabs->addTab(new ZoneTab(map.town, sprites, allFloors(), allElements(), this), "Town");
    tabs->addTab(new ZoneTab(map.mineLevels[0], sprites, allFloors(), allElements(), this), "Dungeon levels");
    tabs->setStyleSheet(TABS_STYLE);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(tabs, 1);
    layout->addWidget(backButton, 0, Qt::AlignRight);

    connect(backButton, &QAbstractButton::clicked, this, &MapEditorScreen::backRequested);
}