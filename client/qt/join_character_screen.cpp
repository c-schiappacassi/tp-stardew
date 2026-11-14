#include "join_character_screen.h"

#include <QGridLayout>
#include <QLabel>
#include <QVBoxLayout>

#include "menu_helpers.h"
#include "qt_constants.h"

namespace {
const QSize fieldSize(400, 90);
}

JoinCharacterScreen::JoinCharacterScreen(QWidget* parent): CharacterScreen(parent) {
    QGridLayout* fieldsLayout = new QGridLayout();
    fieldsLayout->setHorizontalSpacing(10);
    fieldsLayout->setVerticalSpacing(10);
    QLabel* nameLabel = makeLabel("Name", panel);
    nameLabel->setFixedWidth(LABEL_WIDTH);
    fieldsLayout->addWidget(nameLabel, 0, 0);
    fieldsLayout->addWidget(menu.makeInputFrame(nameInput, FIELD_RECT, fieldSize, panel), 0, 1);
    fieldsLayout->addWidget(errorLabel, 1, 1);

    QVBoxLayout* rightColumn = new QVBoxLayout();
    rightColumn->addStretch(1);
    rightColumn->addLayout(fieldsLayout);
    rightColumn->addStretch(1);

    buildLayout(rightColumn);
}

void JoinCharacterScreen::confirm() {
    if (!characterName().isEmpty()) {
        emit confirmed();
    }
}
