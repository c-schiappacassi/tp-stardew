#include "load_screen.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QVBoxLayout>
#include <algorithm>

#include "sprite_button.h"
#include "menu_helpers.h"
#include "qt_constants.h"

namespace {
const QRect upArrowRect(39, 75, 8, 5);
const QRect downArrowRect(48, 75, 8, 5);
const QSize panelSize(1300, 560);
const QSize rowSize(1240, 160);
const QSize arrowSize(64, 40);
const QSize farmerSize(48, 96);

constexpr int rowsPerPage = 3;

QPixmap makeFarmer(const FarmerSprites& sprites, const PlayerData& data) {
    const QPixmap farmer = sprites.drawCharacter(data.hairModel, QColor(data.hairColor.r, data.hairColor.g, data.hairColor.b),
            data.shirtModel, data.pantsModel, QColor(data.pantsColor.r, data.pantsColor.g, data.pantsColor.b));
    return farmer.scaled(farmerSize, Qt::IgnoreAspectRatio, Qt::FastTransformation);
}

QLabel* makeLabel(const QString& text, int size, const QString& color, QWidget* parent) {
    QLabel* label = new QLabel(text, parent);
    label->setStyleSheet(QString("font: bold %1px; color: %2;").arg(size).arg(color));
    label->setAttribute(Qt::WA_TransparentForMouseEvents);
    return label;
}
}

LoadScreen::LoadScreen(QWidget* parent): QWidget(parent), frames(MENUS_PATH + "frames.png") {
    const MenuHelpers menu;

    QLabel* panel = menu.makePanel(PANEL_RECT, panelSize, this);

    rowsLayout = new QVBoxLayout();
    rowsLayout->setSpacing(10);

    QVBoxLayout* panelLayout = new QVBoxLayout(panel);
    panelLayout->setContentsMargins(30, 30, 30, 30);
    panelLayout->addLayout(rowsLayout);

    const QPixmap chatBox(MENUS_PATH + "chat_box.png");
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

    SpriteButton* backButton = menu.makeBackButton(this);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addStretch(2);
    layout->addLayout(panelRow);
    layout->addStretch(2);
    layout->addWidget(backButton, 0, Qt::AlignRight);

    connect(upButton, &QAbstractButton::clicked, this, [this]() {
        firstRow = std::max(0, firstRow - 1);
        showRows();
    });
    connect(downButton, &QAbstractButton::clicked, this, [this]() {
        if (firstRow + rowsPerPage < static_cast<int>(games.size())) {
            ++firstRow;
            showRows();
        }
    });
    connect(backButton, &QAbstractButton::clicked, this, &LoadScreen::backRequested);
}

void LoadScreen::setGames(const std::vector<GameInfoDTO>& newGames) {
    games = newGames;
    firstRow = 0;
    showRows();
}

SpriteButton* LoadScreen::makeRow(const GameInfoDTO& game) {
    SpriteButton* row = new SpriteButton(frames.copy(FIELD_RECT), frames.copy(ROW_HOVER_RECT), this);
    row->setFixedSize(rowSize);

    QLabel* farmer = new QLabel(row);
    farmer->setPixmap(makeFarmer(sprites, game.playerData));
    farmer->setAttribute(Qt::WA_TransparentForMouseEvents);

    QLabel* characterName = makeLabel(QString::fromStdString(game.playerData.characterName), 36, TEXT_COLOR, row);
    QLabel* farmName = makeLabel(QString::fromStdString(game.farmName), 34, TEXT_COLOR, row);
    QLabel* mode = makeLabel(game.isMultiplayer ? "Multiplayer" : "Singleplayer", 26, LIGHTER_TEXT_COLOR, row);

    QVBoxLayout* rightColumn = new QVBoxLayout();
    rightColumn->setContentsMargins(0, 25, 0, 30);
    rightColumn->setSpacing(0);
    rightColumn->addStretch(1);
    rightColumn->addWidget(farmName, 0, Qt::AlignRight);
    rightColumn->addWidget(mode, 0, Qt::AlignRight);
    rightColumn->addStretch(1);

    QHBoxLayout* rowLayout = new QHBoxLayout(row);
    rowLayout->setContentsMargins(70, 0, 60, 0);
    rowLayout->setSpacing(30);
    rowLayout->addWidget(farmer);
    rowLayout->addWidget(characterName);
    rowLayout->addStretch(1);
    rowLayout->addLayout(rightColumn);

    const std::string id = game.id;
    connect(row, &QAbstractButton::clicked, this, [this, id]() { emit resumeRequested(id); });
    return row;
}

void LoadScreen::showRows() {
    while (QLayoutItem* item = rowsLayout->takeAt(0)) {
        delete item->widget();
        delete item;
    }

    if (games.empty()) {
        QLabel* empty = new QLabel("No saved games", this);
        empty->setAlignment(Qt::AlignCenter);
        empty->setStyleSheet(QString("font: bold 40px; color: %1;").arg(TEXT_COLOR));
        rowsLayout->addWidget(empty);
        return;
    }

    const int last = std::min(firstRow + rowsPerPage, static_cast<int>(games.size()));
    for (int i = firstRow; i < last; ++i) {
        rowsLayout->addWidget(makeRow(games[i]));
    }
}