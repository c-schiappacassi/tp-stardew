#include "map_editor_screen.h"

#include <QPixmap>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QDir>

#include "editor_constants.h"
#include "sprite_button.h"
#include "zone/zone_tab.h"
#include "model/zone_options.h"
#include "model/map_wriiter.h"

namespace {
const QRect backNormalRect(296, 252, 66, 28);
const QRect backHoverRect(296, 280, 66, 26);
const QSize backSize(160, 68);
}

MapEditorScreen::MapEditorScreen(QWidget* parent): QWidget(parent) {
    const QPixmap titleSheet(MENUS_PATH + "title_screen.png");
    SpriteButton* backButton = new SpriteButton(titleSheet.copy(backNormalRect), titleSheet.copy(backHoverRect), this);
    backButton->setFixedSize(backSize);

    QTabWidget* tabs = new QTabWidget(this);
    const ZoneOptions farm = farmOptions();
    const ZoneOptions town = townOptions();
    const ZoneOptions mine = mineOptions();
    tabs->addTab(new ZoneTab(map.farm, sprites, farm.floors, farm.elements, this), "Farm");
    tabs->addTab(new ZoneTab(map.town, sprites, town.floors, town.elements, this), "Town");

    QTabWidget* levels = new QTabWidget(this);
    levels->setTabPosition(QTabWidget::South);
    for (std::size_t i = 0; i < map.mineLevels.size(); ++i) {
        levels->addTab(new ZoneTab(map.mineLevels[i], sprites, mine.floors, mine.elements, this), QString("Level %1").arg(i + 1));
    }
    tabs->addTab(levels, "Dungeon levels");
    tabs->setStyleSheet(TABS_STYLE);

    QPushButton* saveButton = new QPushButton("SAVE", this);
    saveButton->setStyleSheet(BUTTON_STYLE);

    QHBoxLayout* buttons = new QHBoxLayout();
    buttons->addStretch(1);
    buttons->addWidget(saveButton);
    buttons->addWidget(backButton);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(tabs, 1);
    layout->addLayout(buttons);

    connect(saveButton, &QPushButton::clicked, this, &MapEditorScreen::saveMap);
    connect(backButton, &QAbstractButton::clicked, this, &MapEditorScreen::backRequested);
}

void MapEditorScreen::saveMap() {
    QDir().mkpath(CUSTOM_MAPS_PATH);
    QString path = QFileDialog::getSaveFileName(this, "Save map", CUSTOM_MAPS_PATH, "TOML map (*.toml)");
    if (path.isEmpty()) {
        return;
    }
    if (!path.endsWith(".toml")) {
        path += ".toml";
    }
    if (!writeMap(map, path.toStdString())) {
        QMessageBox::warning(this, "Save map", "Could not save the map in " + path);
    }
}