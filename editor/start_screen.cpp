#include "start_screen.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QVBoxLayout>

#include "editor_constants.h"
#include "sprite_button.h"

namespace {
const QRect logoRect(0, 0, 400, 187);
const QSize logoSize(960, 449);

const QSize spriteSize(134, 79);
const QSize buttonSize(344, 203);

SpriteButton* makeButton(const QPixmap& sheet, int column, QWidget* parent) {
    const QPoint normalPos(column * spriteSize.width(), 0);
    const QPoint hoverPos(column * spriteSize.width(), spriteSize.height());
    auto* button = new SpriteButton(sheet.copy(QRect(normalPos, spriteSize)),
                                    sheet.copy(QRect(hoverPos, spriteSize)), parent);
    button->setFixedSize(buttonSize);
    return button;
}
}

StartScreen::StartScreen(QWidget* parent): QWidget(parent) {
    const QPixmap titleSheet(MENUS_PATH + "title_screen.png");
    const QPixmap buttonsSheet(EDITOR_PATH + "menu_buttons.png");

    auto* logo = new QLabel(this);
    logo->setPixmap(titleSheet.copy(logoRect));
    logo->setScaledContents(true);
    logo->setFixedSize(logoSize);

    SpriteButton* createButton = makeButton(buttonsSheet, 0, this);
    SpriteButton* editButton = makeButton(buttonsSheet, 1, this);

    auto* buttonsRow = new QHBoxLayout();
    buttonsRow->setSpacing(24);
    buttonsRow->addStretch();
    buttonsRow->addWidget(createButton);
    buttonsRow->addWidget(editButton);
    buttonsRow->addStretch();

    auto* layout = new QVBoxLayout(this);
    layout->addStretch(1);
    layout->addWidget(logo, 0, Qt::AlignHCenter);
    layout->addStretch(6);
    layout->addLayout(buttonsRow);
    layout->addStretch(1);

    connect(createButton, &QAbstractButton::clicked, this, &StartScreen::createMapRequested);
    connect(editButton, &QAbstractButton::clicked, this, &StartScreen::editMapRequested);
}
