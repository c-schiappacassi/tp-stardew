#include "startmenuscreen.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

#include "spritebutton.h"

namespace {
const QRect logoRect(0, 0, 400, 187);
const QSize buttonSize(74, 55);
const QSize logoSize(960, 449);
const QSize scaledButtonSize(190, 141);
constexpr int normalRow = 188;
constexpr int hoverRow = 246;


SpriteButton* makeButton(const QPixmap& source, int column, QWidget* parent) {
    const int x = column * buttonSize.width();
    const QPixmap normal = source.copy(QRect(QPoint(x, normalRow), buttonSize));
    const QPixmap hover = source.copy(QRect(QPoint(x, hoverRow), buttonSize));
    SpriteButton* button = new SpriteButton(normal, hover, parent);
    button->setFixedSize(scaledButtonSize);
    return button;
}
}

StartMenuScreen::StartMenuScreen(QWidget* parent): QWidget(parent) {
    const QPixmap sheet(QString(ASSETS_DIR) + "/Sprites/Menus/Title Screen.png");
    QLabel* logo = new QLabel(this);
    logo->setPixmap(sheet.copy(logoRect));
    logo->setScaledContents(true);
    logo->setFixedSize(logoSize);

    SpriteButton* newButton = makeButton(sheet, 0, this);
    SpriteButton* loadButton = makeButton(sheet, 1, this);
    SpriteButton* joinButton = makeButton(sheet, 2, this);
    SpriteButton* exitButton = makeButton(sheet, 3, this);    

    auto* buttonsRow = new QHBoxLayout();
    buttonsRow->setSpacing(24);
    buttonsRow->addStretch();
    buttonsRow->addWidget(newButton);
    buttonsRow->addWidget(loadButton);
    buttonsRow->addWidget(joinButton);
    buttonsRow->addWidget(exitButton);
    buttonsRow->addStretch();

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addStretch(1);
    layout->addWidget(logo, 0, Qt::AlignHCenter);
    layout->addStretch(6);
    layout->addLayout(buttonsRow);
    layout->addStretch(1);

    connect(newButton, &QAbstractButton::clicked, this, &StartMenuScreen::newGameRequested);
    connect(loadButton, &QAbstractButton::clicked, this, &StartMenuScreen::loadGameRequested);
    connect(joinButton, &QAbstractButton::clicked, this, &StartMenuScreen::joinGameRequested);
    connect(exitButton, &QAbstractButton::clicked, this, &StartMenuScreen::exitRequested);
}
