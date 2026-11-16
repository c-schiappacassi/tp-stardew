#ifndef MAP_EDITOR_SCREEN_H
#define MAP_EDITOR_SCREEN_H

#include <QWidget>

class MapEditorScreen : public QWidget {
    Q_OBJECT

public:
    explicit MapEditorScreen(QWidget* parent = nullptr);

signals:
    void backRequested();
};

#endif  // MAP_EDITOR_SCREEN_H
