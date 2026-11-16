#ifndef SIDE_BAR_H
#define SIDE_BAR_H

#include <vector>

#include <QListWidget>

#include "../model/catalog.h"
#include "sprites.h"

class SideBar : public QListWidget {
    Q_OBJECT

public:
    SideBar(const Sprites& sprites, const std::vector<TileTextureId>& floors, const std::vector<ElementId>& elements, QWidget* parent = nullptr);

private:
    void addHeader(const QString& text);
    void addEntry(const QString& text, const QPixmap& icon);
    void addElements(const Sprites& sprites, const std::vector<ElementId>& elements, ElementKind kind, const QString& header);
};

#endif  //SIDE_BAR_H