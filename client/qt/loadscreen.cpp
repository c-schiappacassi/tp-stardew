#include "loadscreen.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QVBoxLayout>
#include <QImage>
#include <QPainter>
#include <algorithm>

#include "spritebutton.h"

namespace {
const QRect panelRect(0, 0, 440, 260);
const QRect rowRect(0, 620, 440, 100);
const QRect rowHoverRect(0, 760, 440, 100);
const QRect upArrowRect(39, 75, 8, 5);
const QRect downArrowRect(48, 75, 8, 5);
const QRect backNormalRect(296, 252, 66, 28);
const QRect backHoverRect(296, 280, 66, 26);
const QSize panelSize(1300, 560);
const QSize rowSize(1240, 160);
const QSize arrowSize(64, 40);
const QSize backSize(200, 85);
const QSize farmerSize(48, 96);

constexpr int rowsPerPage = 3;

const QString textColor = "#4a2a0a";
const QString lighterTextColor = "#8a5a2a";

struct SheetLayout {
    int perRow;
    int stepX;
    int stepY;
    int cellWidth;
    int cellHeight;

    QRect cell(int model) const {
        return QRect((model % perRow) * stepX, (model / perRow) * stepY, cellWidth, cellHeight);
    }
};

const SheetLayout hairLayout{16, 16, 96, 16, 32};
const SheetLayout shirtLayout{16, 8, 32, 8, 8};
const SheetLayout pantsLayout{10, 192, 688, 16, 32};
const QRect bodyRect(0, 0, 16, 32);
const QRect armsRect(96, 0, 16, 32);
const QPoint shirtOffset(4, 15);
const QPoint hairOffset(0, 1);

QPixmap tinted(const QPixmap& sprite, const QColor& color) {
    QImage image = sprite.toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);
    QPainter painter(&image);
    painter.setCompositionMode(QPainter::CompositionMode_Multiply);
    painter.fillRect(image.rect(), color);
    painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    painter.drawPixmap(0, 0, sprite);
    painter.end();
    return QPixmap::fromImage(image);
}

QPixmap makeFarmer(const PlayerData& data) {
    const QString characters = QString(ASSETS_DIR) + "/Sprites/Characters/";
    const QPixmap farmerSheet(characters + "Farmer Base.png");
    const QPixmap hairSheet(characters + "Hairstyles.png");
    const QPixmap shirtSheet(characters + "Shirts.png");
    const QPixmap pantsSheet(characters + "Pants.png");

    QPixmap farmer(bodyRect.size());
    farmer.fill(Qt::transparent);
    QPainter painter(&farmer);
    painter.drawPixmap(0, 0, farmerSheet.copy(bodyRect));
    painter.drawPixmap(0, 0, tinted(pantsSheet.copy(pantsLayout.cell(data.pantsModel)),
            QColor(data.pantsColor.r, data.pantsColor.g, data.pantsColor.b)));
    painter.drawPixmap(shirtOffset, shirtSheet.copy(shirtLayout.cell(data.shirtModel)));
    painter.drawPixmap(hairOffset, tinted(hairSheet.copy(hairLayout.cell(data.hairModel)),
            QColor(data.hairColor.r, data.hairColor.g, data.hairColor.b)));
    painter.drawPixmap(0, 0, farmerSheet.copy(armsRect));
    return farmer.scaled(farmerSize, Qt::IgnoreAspectRatio, Qt::FastTransformation);
}

QLabel* makeLabel(const QString& text, int size, const QString& color, QWidget* parent) {
    QLabel* label = new QLabel(text, parent);
    label->setStyleSheet(QString("font: bold %1px; color: %2;").arg(size).arg(color));
    label->setAttribute(Qt::WA_TransparentForMouseEvents);
    return label;
}
}

LoadScreen::LoadScreen(QWidget* parent): QWidget(parent), frames(QString(ASSETS_DIR) + "/Sprites/Menus/Frames.png") {
    QLabel* panel = new QLabel(this);
    panel->setPixmap(frames.copy(panelRect));
    panel->setScaledContents(true);
    panel->setFixedSize(panelSize);

    rowsLayout = new QVBoxLayout();
    rowsLayout->setSpacing(10);

    QVBoxLayout* panelLayout = new QVBoxLayout(panel);
    panelLayout->setContentsMargins(30, 30, 30, 30);
    panelLayout->addLayout(rowsLayout);

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
    SpriteButton* row = new SpriteButton(frames.copy(rowRect), frames.copy(rowHoverRect), this);
    row->setFixedSize(rowSize);

    QLabel* farmer = new QLabel(row);
    farmer->setPixmap(makeFarmer(game.playerData));
    farmer->setAttribute(Qt::WA_TransparentForMouseEvents);

    QLabel* characterName = makeLabel(QString::fromStdString(game.playerData.characterName), 36, textColor, row);
    QLabel* farmName = makeLabel(QString::fromStdString(game.farmName), 34, textColor, row);
    QLabel* mode = makeLabel(game.isMultiplayer ? "Multiplayer" : "Singleplayer", 26, lighterTextColor, row);

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

    const uint16_t id = game.id;
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
        empty->setStyleSheet(QString("font: bold 40px; color: %1;").arg(textColor));
        rowsLayout->addWidget(empty);
        return;
    }

    const int last = std::min(firstRow + rowsPerPage, static_cast<int>(games.size()));
    for (int i = firstRow; i < last; ++i) {
        rowsLayout->addWidget(makeRow(games[i]));
    }
}