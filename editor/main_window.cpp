#include "main_window.h"

#include <QStackedWidget>

#include "editor_constants.h"
#include "create_map_screen.h"
#include "start_screen.h"

MainWindow::MainWindow(QWidget* parent): QMainWindow(parent), stack(new QStackedWidget(this)) {
    setWindowTitle("Stardew Valley - Editor de mapas");
    setMinimumSize(1600, 900);
    resize(1920, 1200);
    setStyleSheet(QString("MainWindow { border-image: url(\"%1day_sky.png\") 0 0 0 0 stretch stretch; }").arg(MENUS_PATH));
    setCentralWidget(stack);

    StartScreen* startScreen = new StartScreen(this);
    CreateMapScreen* createMapScreen = new CreateMapScreen(this);

    stack->addWidget(startScreen);
    stack->addWidget(createMapScreen);

    connect(startScreen, &StartScreen::createMapRequested, this, [this, createMapScreen]() { stack->setCurrentWidget(createMapScreen); });
    //TODO: editMapRequested -> selector de mapas y cargarlo
    connect(createMapScreen, &CreateMapScreen::backRequested, this, [this, startScreen]() { stack->setCurrentWidget(startScreen); });
}
