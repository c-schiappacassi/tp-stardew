#ifndef MAP_EDITOR_SCREEN_H
#define MAP_EDITOR_SCREEN_H

#include <QWidget>
#include "model/world_map.h"
#include "zone/sprites.h"

class MapEditorScreen : public QWidget {
    Q_OBJECT

public:
    explicit MapEditorScreen(const WorldMap& startingMap, QWidget* parent = nullptr);

signals:
    void backRequested();

private:
    void saveMap();
    Sprites sprites;
    WorldMap map;
};

#endif  // MAP_EDITOR_SCREEN_H
