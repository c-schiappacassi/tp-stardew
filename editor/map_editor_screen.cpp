#include "map_editor_screen.h"

#include <QPushButton>
#include <QTabWidget>
#include <QVBoxLayout>

#include "editor_constants.h"

MapEditorScreen::MapEditorScreen(QWidget* parent): QWidget(parent) {
    QPushButton* backButton = new QPushButton("BACK", this);
    backButton->setFixedSize(200, 60);

    QTabWidget* tabs = new QTabWidget(this);
    tabs->addTab(new QWidget(this), "Farm");
    tabs->addTab(new QWidget(this), "Town");
    tabs->addTab(new QWidget(this), "Dungeon levels");
    tabs->setStyleSheet(TABS_STYLE);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(tabs, 1);
    layout->addWidget(backButton, 0, Qt::AlignRight);

    connect(backButton, &QPushButton::clicked, this, &MapEditorScreen::backRequested);
}