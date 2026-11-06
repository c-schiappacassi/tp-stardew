#include "main_window.h"

#include <QStackedWidget>
#include <QApplication>
#include <QMessageBox>

#include "start_menu_screen.h"
#include "username_screen.h"
#include "join_screen.h"
#include "load_screen.h"
#include "new_game_screen.h"
#include "join_character_screen.h"
#include "network.h"

MainWindow::MainWindow(NetworkManager& net, QWidget *parent) : QMainWindow(parent), stack(new QStackedWidget(this)), net(net) {
    setWindowTitle("Stardew Valley");
    setMinimumSize(1600, 900);
    resize(1920, 1200);
    setStyleSheet(QString("MainWindow { border-image: url(\"%1/sprites/menus/day_sky.png\") 0 0 0 0 stretch stretch; }").arg(ASSETS_DIR));
    setCentralWidget(stack);

    UsernameScreen* usernameScreen = new UsernameScreen(this);
    StartMenuScreen* startMenu = new StartMenuScreen(this);
    JoinScreen* joinScreen = new JoinScreen(this);
    JoinCharacterScreen* joinCharacterScreen = new JoinCharacterScreen(this);
    LoadScreen* loadScreen = new LoadScreen(this);
    NewGameScreen* newGameScreen = new NewGameScreen(this);

    stack->addWidget(usernameScreen);
    stack->addWidget(startMenu);
    stack->addWidget(joinScreen);
    stack->addWidget(joinCharacterScreen);
    stack->addWidget(loadScreen);
    stack->addWidget(newGameScreen);

    connect(usernameScreen, &UsernameScreen::usernameConfirmed, this, [this, usernameScreen, startMenu](const QString& name) {
        const std::string user = name.toStdString();
        const qtStatus reply = sendQt(ListGamesDTO(user));
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
        const qtStatus reply = sendQt(ListGamesDTO(username));
        if (reply.status != StatusCode::Ok) {
            QMessageBox::warning(this, "Servidor", QString::fromStdString(reply.errorMsg));
            return;
        }
        games = reply.games;
        loadScreen->setGames(games);
        stack->setCurrentWidget(loadScreen);
    });

    connect(loadScreen, &LoadScreen::resumeRequested, this, [this](const std::string& gameId) {
        ResumeGameDTO dto(username);
        dto.gameId = gameId;
        const ServerStatus reply = send(dto);
        if (reply.status == StatusCode::Ok) {
            enterGame(reply);
        } else {
            QMessageBox::warning(this, "Servidor", QString::fromStdString(reply.errorMsg));
        }
    });

    connect(startMenu, &StartMenuScreen::newGameRequested, this, [this, newGameScreen]() {
        stack->setCurrentWidget(newGameScreen);
    });

    connect(joinScreen, &JoinScreen::joinRequested, this, [this, joinScreen, joinCharacterScreen](const QString& code) {
        CheckCodeDTO dto(username);
        dto.code = code.toStdString();
        const qtStatus reply = sendQt(dto);
        if (reply.status == StatusCode::Ok) {
            joinCode = dto.code;
            stack->setCurrentWidget(joinCharacterScreen);
        } else {
            joinScreen->showError(QString::fromStdString(reply.errorMsg));
        }
    });

    connect(joinCharacterScreen, &JoinCharacterScreen::confirmed, this, [this, joinCharacterScreen]() {
        JoinGameDTO dto(username);
        dto.characterName = joinCharacterScreen->characterName().toStdString();
        dto.code = joinCode;
        dto.hairModel = joinCharacterScreen->hairModel();
        dto.shirtModel = joinCharacterScreen->shirtModel();
        dto.pantsModel = joinCharacterScreen->pantsModel();
        dto.hairColor = joinCharacterScreen->hairColor();
        dto.pantsColor = joinCharacterScreen->pantsColor();

        const ServerStatus reply = send(dto);
        if (reply.status == StatusCode::Ok) {
            enterGame(reply);
        } else {
            joinCharacterScreen->showError(QString::fromStdString(reply.errorMsg));
        }
    });

    connect(joinCharacterScreen, &JoinCharacterScreen::backRequested, this, [this, joinScreen]() {
        stack->setCurrentWidget(joinScreen);
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

    connect(newGameScreen, &NewGameScreen::confirmed, this, [this, newGameScreen]() {
        CreateGameDTO dto(username);
        dto.characterName = newGameScreen->characterName().toStdString();
        dto.gameName = newGameScreen->farmName().toStdString();
        dto.worldId = 0;
        dto.hairModel = newGameScreen->hairModel();
        dto.shirtModel = newGameScreen->shirtModel();
        dto.pantsModel = newGameScreen->pantsModel();
        dto.hairColor = newGameScreen->hairColor();
        dto.pantsColor = newGameScreen->pantsColor();

        const ServerStatus reply = send(dto);
        if (reply.status == StatusCode::Ok) {
            enterGame(reply);
        } else {
            newGameScreen->showError(QString::fromStdString(reply.errorMsg));
        }
    });

}

void MainWindow::enterGame(const ServerStatus& status) {
    entryStatus = status;
    hide();
    QApplication::quit();
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

qtStatus MainWindow::sendQt(const CommandDTO& dto) {
    try {
        return net.handshakeQt(dto);
    } catch (const std::exception&) {
        qtStatus failed{};
        failed.status = StatusCode::Error;
        failed.errorMsg = "Could not connect to the server";
        return failed;
    }
}
