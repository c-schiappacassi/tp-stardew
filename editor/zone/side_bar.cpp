#include "side_bar.h"

#include <QPainter>

namespace {
const QSize ICON_SIZE(48, 48);
const int SIDE_BAR_WIDTH = 260;
}

SideBar::SideBar(const Sprites& sprites, const std::vector<TileTextureId>& floors, const std::vector<ElementId>& elements, QWidget* parent): QListWidget(parent) {
    setIconSize(ICON_SIZE);
    setFixedWidth(SIDE_BAR_WIDTH);

    if (!floors.empty()) {
        addHeader("Floors");
        for (TileTextureId floor: floors) {
            addEntry(QString::fromStdString(FloorCatalog::infoOf(floor).name), sprites.floor(floor), {Tool::Kind::PaintFloor, floor, {}});        }
    }
    addElements(sprites, elements, ElementKind::Building, "Buildings");
    addElements(sprites, elements, ElementKind::Decor, "Decor");
    addElements(sprites, elements, ElementKind::Portal, "Portals");

    addHeader("Tools");
    addEntry("Select / move", sprites.moveIcon(), {Tool::Kind::Move, {}, {}});
    addEntry("Erase", sprites.eraseIcon(), {Tool::Kind::Erase, {}, {}});

    connect(this, &QListWidget::currentRowChanged, this, [this](int row) {
        const QVariant index = item(row)->data(Qt::UserRole);
        if (index.isValid()) {
            emit toolSelected(tools[static_cast<std::size_t>(index.toInt())]);
        }
    });
}

void SideBar::addHeader(const QString& text) {
    QListWidgetItem* item = new QListWidgetItem(text, this);
    item->setFlags(Qt::NoItemFlags);
}

void SideBar::addEntry(const QString& text, const QPixmap& icon, const Tool& tool) {    const QPixmap scaled = icon.scaled(ICON_SIZE, Qt::KeepAspectRatio, Qt::FastTransformation);
    QPixmap square(ICON_SIZE);
    square.fill(Qt::transparent);
    QPainter painter(&square);
    painter.drawPixmap((ICON_SIZE.width() - scaled.width()) / 2, (ICON_SIZE.height() - scaled.height()) / 2, scaled);
    painter.end();

    QListWidgetItem* item = new QListWidgetItem(text, this);
    item->setIcon(QIcon(square));

    item->setData(Qt::UserRole, static_cast<int>(tools.size()));
    tools.push_back(tool);
}

void SideBar::addElements(const Sprites& sprites, const std::vector<ElementId>& elements, ElementKind kind, const QString& header) {
    bool headerAdded = false;
    for (ElementId element: elements) {
        if (element.kind != kind) {
            continue;
        }
        if (!headerAdded) {
            addHeader(header);
            headerAdded = true;
        }
        addEntry(QString::fromStdString(ElementCatalog::infoOf(element).name), sprites.element(element), {Tool::Kind::PlaceElement, {}, element});    }
}