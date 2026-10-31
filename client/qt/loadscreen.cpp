#include "loadscreen.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QTransform>
#include <QVBoxLayout>

#include "spritebutton.h"

namespace {
const QSize panelSize(1300, 560);
const QSize arrowSize(64, 40);
const QRect upArrowRect(39, 75, 8, 5);
const QRect downArrowRect(48, 75, 8, 5);
const QRect backNormalRect(296, 252, 66, 28);
const QRect backHoverRect(296, 280, 66, 26);
const QSize backSize(200, 85);

const QRect panelRect(1200, 51, 175, 201);
}

LoadScreen::LoadScreen(QWidget* parent): QWidget(parent) {
    const QPixmap frame = QPixmap(QString(ASSETS_DIR) + "/Sprites/Menus/FQMenu.png").copy(panelRect).transformed(QTransform().rotate(90));

    QLabel* panel = new QLabel(this);
    panel->setPixmap(frame);
    panel->setScaledContents(true);
    panel->setFixedSize(panelSize);

    const QPixmap chatBox(QString(ASSETS_DIR) + "/Sprites/Menus/Chat Box.png");
    SpriteButton* upButton = new SpriteButton(chatBox.copy(upArrowRect), chatBox.copy(upArrowRect), this);
    upButton->setFixedSize(arrowSize);
    SpriteButton* downButton = new SpriteButton(chatBox.copy(downArrowRect), chatBox.copy(downArrowRect), this);
    downButton->setFixedSize(arrowSize);

    QVBoxLayout* arrowsLayout = new QVBoxLayout();
    arrowsLayout->addWidget(upButton);
    arrowsLayout->addStretch(1);
    arrowsLayout->addWidget(downButton);

    QHBoxLayout* panelRow = new QHBoxLayout();
    panelRow->addStretch(1);
    panelRow->addSpacing(arrowSize.width() + 20);
    panelRow->addWidget(panel);
    panelRow->addSpacing(20);
    panelRow->addLayout(arrowsLayout);
    panelRow->addStretch(1);

    const QPixmap titleSheet(QString(ASSETS_DIR) + "/Sprites/Menus/Title Screen.png");
    SpriteButton* backButton = new SpriteButton(titleSheet.copy(backNormalRect), titleSheet.copy(backHoverRect), this);
    backButton->setFixedSize(backSize);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addStretch(2);
    layout->addLayout(panelRow);
    layout->addStretch(2);
    layout->addWidget(backButton, 0, Qt::AlignRight);

    connect(backButton, &QAbstractButton::clicked, this, &LoadScreen::backRequested);
}