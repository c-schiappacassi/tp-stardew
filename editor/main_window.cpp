#include "main_window.h"

#include <QDir>
#include <QFileDialog>
#include <QStackedWidget>

#include "editor_constants.h"
#include "map_editor_screen.h"
#include "model/map_reader.h"
#include "start_screen.h"

MainWindow::MainWindow(QWidget* parent): QMainWindow(parent), stack(new QStackedWidget(this)), startScreen(new StartScreen(this)) {
    setWindowTitle("Stardew Valley - Editor de mapas");
    setMinimumSize(1600, 900);
    resize(1920, 1200);
    setStyleSheet(QString("MainWindow { border-image: url(\"%1day_sky.png\") 0 0 0 0 stretch stretch; }").arg(MENUS_PATH));
    setCentralWidget(stack);

    stack->addWidget(startScreen);

    connect(startScreen, &StartScreen::createMapRequested, this, &MainWindow::createMap);
    connect(startScreen, &StartScreen::editMapRequested, this, &MainWindow::editExistingMap);
}

void MainWindow::openEditor(const WorldMap& map) {
    closeEditor();
    editor = new MapEditorScreen(map, this);
    stack->addWidget(editor);
    stack->setCurrentWidget(editor);
    connect(editor, &MapEditorScreen::backRequested, this, &MainWindow::closeEditor);
}

void MainWindow::createMap() {
    openEditor(readMap(BASE_MAP_PATH.toStdString()));
}

void MainWindow::editExistingMap() {
    QDir().mkpath(CUSTOM_MAPS_PATH);
    const QString path = QFileDialog::getOpenFileName(this, "Edit map", CUSTOM_MAPS_PATH, "TOML map (*.toml)");
    if (path.isEmpty()) {
        return;
    }
    openEditor(readMap(path.toStdString()));
}

void MainWindow::closeEditor() {
    stack->setCurrentWidget(startScreen);
    if (editor) {
        stack->removeWidget(editor);
        editor->deleteLater();
        editor = nullptr;
    }
}
