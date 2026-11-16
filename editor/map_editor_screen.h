#ifndef MAP_EDITOR_SCREEN_H
#define MAP_EDITOR_SCREEN_H

#include <QWidget>
#include "zone/sprites.h"

class MapEditorScreen : public QWidget {
    Q_OBJECT

public:
    explicit MapEditorScreen(QWidget* parent = nullptr);

signals:
    void backRequested();

private:
    Sprites sprites;
};

#endif  // MAP_EDITOR_SCREEN_H
