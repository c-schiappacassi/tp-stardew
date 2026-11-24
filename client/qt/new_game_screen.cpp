#include "new_game_screen.h"

#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>

#include "menu_helpers.h"
#include "qt_constants.h"
#include "map_selector.h"

namespace {
const QSize fieldSize(400, 90);
}

NewGameScreen::NewGameScreen(QWidget* parent): CharacterScreen(parent) {
    farmNameInput = new QLineEdit(panel);
    farmNameInput->setMaxLength(16);
    farmNameInput->setStyleSheet(QString("QLineEdit { background: transparent; border: none; font: bold 32px; color: %1; }")
        .arg(TEXT_COLOR));

    QGridLayout* fieldsLayout = new QGridLayout();
    fieldsLayout->setHorizontalSpacing(10);
    fieldsLayout->setVerticalSpacing(40);
    QLabel* nameLabel = makeLabel("Name", panel);
    nameLabel->setFixedWidth(LABEL_WIDTH);
    QLabel* farmLabel = makeLabel("Farm\nname", panel);
    farmLabel->setFixedWidth(LABEL_WIDTH);
    fieldsLayout->addWidget(nameLabel, 0, 0);
    fieldsLayout->addWidget(menu.makeInputFrame(nameInput, FIELD_RECT, fieldSize, panel), 0, 1);
    fieldsLayout->addWidget(farmLabel, 1, 0);
    fieldsLayout->addWidget(menu.makeInputFrame(farmNameInput, FIELD_RECT, fieldSize, panel), 1, 1);
    fieldsLayout->addWidget(errorLabel, 2, 1);

    QVBoxLayout* rightColumn = new QVBoxLayout();
    rightColumn->addLayout(fieldsLayout);
    rightColumn->addStretch(1);

    mapSelector = new MapSelector(menu, this);
    buildLayout(rightColumn, mapSelector);

    connect(farmNameInput, &QLineEdit::textChanged, this, [this]() { errorLabel->clear(); });
}

void NewGameScreen::confirm() {
    if (!characterName().isEmpty() && !farmName().isEmpty()) {
        emit confirmed();
    }
}

QString NewGameScreen::farmName() const {
    return farmNameInput->text().trimmed();
}

std::string NewGameScreen::mapPath() const {
    return mapSelector->mapPath();
}
