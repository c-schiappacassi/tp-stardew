#include "map_selector.h"

#include <QButtonGroup>
#include <QDir>
#include <QFileInfo>
#include <QInputDialog>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include "menu_helpers.h"
#include "qt_constants.h"
#include "sprite_button.h"

namespace {
const QSize selectorSize(220, 840);
const QSize boxSize(170, 720);
const QSize iconSize(104, 104);
const QSize myMapsSize(200, 70);
const int ICON_STEP = 22;
const int MAX_PLAYERS = 4;
}

MapSelector::MapSelector(const MenuHelpers& menu, QWidget* parent):
        QWidget(parent), defaults(new QButtonGroup(this)), customLabel(nullptr) {
    setFixedSize(selectorSize);
    QLabel* box = menu.makePanel(VERTICAL_PANEL_RECT, boxSize, this);
    box->setStyleSheet(QString(
        "QPushButton { background: transparent; border: 4px solid transparent; padding: 2px; }"
        "QPushButton:hover { border-color: rgba(216, 48, 28, 110); }"
        "QPushButton:checked { border-color: %1; }"
        "QToolTip { background: #f6d7a0; color: %2; border: 2px solid %1; font: bold 20px; }")
        .arg(SELECTED_COLOR, TEXT_COLOR));

    QVBoxLayout* boxLayout = new QVBoxLayout(box);
    boxLayout->setContentsMargins(0, 30, 0, 30);

    const QPixmap cursors(MENUS_PATH + "cursors.png");
    for (int players = 1; players <= MAX_PLAYERS; ++players) {
        QPushButton* button = new QPushButton(box);
        const QRect iconRect = MAP_ICON_RECT.translated((players - 1) * ICON_STEP, 0);
        button->setIcon(QIcon(cursors.copy(iconRect).scaled(iconSize)));
        button->setIconSize(iconSize);
        button->setCheckable(true);
        button->setToolTip(QString("%1 player map").arg(players));
        button->setCursor(Qt::PointingHandCursor);
        defaults->addButton(button, players);
        boxLayout->addStretch(1);
        boxLayout->addWidget(button, 0, Qt::AlignHCenter);
    }
    boxLayout->addStretch(1);
    defaults->button(1)->setChecked(true);

    SpriteButton* myMapsButton = menu.makePushButton("My maps", this, myMapsSize);
    customLabel = new QLabel(this);
    customLabel->setAlignment(Qt::AlignCenter);
    customLabel->setWordWrap(true);
    customLabel->setStyleSheet(QString("font: bold 22px; color: %1;").arg(LIGHTER_TEXT_COLOR));

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);
    layout->addWidget(box, 0, Qt::AlignHCenter);
    layout->addWidget(myMapsButton, 0, Qt::AlignHCenter);
    layout->addWidget(customLabel, 1);

    connect(defaults, &QButtonGroup::idClicked, this, [this]() {
        customMap.clear();
        customLabel->clear();
    });
    connect(myMapsButton, &QAbstractButton::clicked, this, &MapSelector::chooseCustomMap);
}
void MapSelector::chooseCustomMap() {
    const QStringList maps = QDir(CUSTOM_MAPS_PATH).entryList({"*.toml"}, QDir::Files, QDir::Name);
    if (maps.isEmpty()) {
        customLabel->setText("You don't have custom maps yet");
        return;
    }
    bool ok = false;
    const QString chosen = QInputDialog::getItem(this, "My maps", "Choose one of your maps:", maps, 0, false, &ok);
    if (!ok) {
        return;
    }
    customMap = chosen;
    customLabel->setText(QFileInfo(chosen).completeBaseName());

    defaults->setExclusive(false);
    if (QAbstractButton* checked = defaults->checkedButton()) {
        checked->setChecked(false);
    }
    defaults->setExclusive(true);
}

std::string MapSelector::mapPath() const {
    if (!customMap.isEmpty()) {
        return "maps/custom/" + customMap.toStdString();
    }
    return "maps/predefined/players_" + std::to_string(defaults->checkedId()) + ".toml";
}