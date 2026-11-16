#include "zone_tab.h"

#include <QHBoxLayout>

#include "side_bar.h"

ZoneTab::ZoneTab(const Sprites& sprites, const std::vector<TileTextureId>& floors, const std::vector<ElementId>& elements, QWidget* parent): QWidget(parent) {
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->addWidget(new SideBar(sprites, floors, elements, this));
    layout->addStretch(1);
}