#pragma once

#include <QString>
#include <QWidget>
#include "model/world_map.h"
#include "zone/sprites.h"

class MapEditorScreen : public QWidget {
    Q_OBJECT

public:
    MapEditorScreen(const WorldMap& startingMap, const QString& filePath, QWidget* parent = nullptr);

signals:
    void backRequested();

private:
    void saveMap();
    Sprites sprites;
    WorldMap map;
    QString filePath;
};
