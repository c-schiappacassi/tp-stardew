#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>

#include "model/world_map.h"

class QStackedWidget;
class MapEditorScreen;
class StartScreen;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    void openEditor(const WorldMap& map);
    void createMap();
    void editExistingMap();
    void closeEditor();

    QStackedWidget* stack;
    StartScreen* startScreen;
    MapEditorScreen* editor = nullptr;
};

#endif  // MAIN_WINDOW_H
