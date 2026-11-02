#include "mainwindow.h"

#include <QStackedWidget>

#include "startmenuscreen.h"
#include "usernamescreen.h"
#include "joinscreen.h"
#include "loadscreen.h"
#include "newgamescreen.h"
#include "network.h"

MainWindow::MainWindow(NetworkManager& net, QWidget *parent) : QMainWindow(parent), stack(new QStackedWidget(this)), net(net) {
    setWindowTitle("Stardew Valley");
    setMinimumSize(1600, 900);
    resize(1920, 1200);
    setStyleSheet(QString("MainWindow { border-image: url(\"%1/Sprites/Menus/DaySky.png\") 0 0 0 0 stretch stretch; }").arg(ASSETS_DIR));
    setCentralWidget(stack);

    UsernameScreen* usernameScreen = new UsernameScreen(this);
    StartMenuScreen* startMenu = new StartMenuScreen(this);
    JoinScreen* joinScreen = new JoinScreen(this);
    LoadScreen* loadScreen = new LoadScreen(this);
    NewGameScreen* newGameScreen = new NewGameScreen(this);
    stack->addWidget(usernameScreen);
    stack->addWidget(startMenu);
    stack->addWidget(joinScreen);
    stack->addWidget(loadScreen);
    stack->addWidget(newGameScreen);

    connect(usernameScreen, &UsernameScreen::usernameConfirmed, this, [this, usernameScreen, startMenu](const QString& name) {
        const std::string user = name.toStdString();
        const ServerStatus reply = send(RequestGameListDTO(user));
        if (reply.status == StatusCode::Ok) {
            username = user;
            games = reply.games;
            stack->setCurrentWidget(startMenu);
        } else {
            usernameScreen->showError(QString::fromStdString(reply.errorMsg));
        }
    });
    connect(startMenu, &StartMenuScreen::exitRequested, this, &QWidget::close);
    connect(startMenu, &StartMenuScreen::joinGameRequested, this, [this, joinScreen]() {
        stack->setCurrentWidget(joinScreen);
    });
    connect(startMenu, &StartMenuScreen::loadGameRequested, this, [this, loadScreen]() {
        stack->setCurrentWidget(loadScreen);
    });
    connect(startMenu, &StartMenuScreen::newGameRequested, this, [this, newGameScreen]() {
        stack->setCurrentWidget(newGameScreen);
    });
    connect(joinScreen, &JoinScreen::backRequested, this, [this, startMenu]() {
        stack->setCurrentWidget(startMenu);
    });
    connect(loadScreen, &LoadScreen::backRequested, this, [this, startMenu]() {
        stack->setCurrentWidget(startMenu);
    });
    connect(newGameScreen, &NewGameScreen::backRequested, this, [this, startMenu]() {
        stack->setCurrentWidget(startMenu);
    });

}

ServerStatus MainWindow::send(const CommandDTO& dto) {
    try {
        return net.handshake(dto);
    } catch (const std::exception&) {
        ServerStatus failed{};
        failed.status = StatusCode::Error;
        failed.errorMsg = "Could not connect to the server";
        return failed;
    }
}
