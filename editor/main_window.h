#pragma once

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
    void openEditor(const WorldMap& map, const QString& savePath);
    void openMapFile(const QString& path, const QString& savePath);
    void createMap();
    void editExistingMap();
    void closeEditor();

    QStackedWidget* stack;
    StartScreen* startScreen;
    MapEditorScreen* editor = nullptr;
};
