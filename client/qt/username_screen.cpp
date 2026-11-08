#include "username_screen.h"

#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>

#include "sprite_button.h"
#include "menu_helpers.h"
#include "qt_constants.h"

namespace {
const QSize panelSize(768, 304);
const QSize fieldSize(480, 100);
const QSize buttonSize(320, 80);
const QSize errorSize(640, 80);
}

UsernameScreen::UsernameScreen(QWidget* parent): QWidget(parent), input(new QLineEdit(this)) {
    const MenuHelpers menu;

    QLabel* logo = menu.makeLogo(this);

    QLabel* panel = menu.makePanel(POST_PANEL_RECT, panelSize, this);

    QLabel* title = new QLabel("Enter username", panel);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(QString("font: bold 40px; color: %1;").arg(TEXT_COLOR));

    input->setAlignment(Qt::AlignCenter);
    input->setMaxLength(16);
    input->setStyleSheet(QString("QLineEdit { background: transparent; border: none; font: bold 38px; color: %1; }")
        .arg(TEXT_COLOR));

    QLabel* field = menu.makeInputFrame(input, FIELD_RECT, fieldSize, panel);

    errorLabel = new QLabel(panel);
    errorLabel->setFixedSize(errorSize);
    errorLabel->setWordWrap(true);
    errorLabel->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    errorLabel->setStyleSheet(QString("font: bold 26px; color: %1;").arg(ERROR_COLOR));

    QVBoxLayout* panelLayout = new QVBoxLayout(panel);
    panelLayout->addStretch(1);
    panelLayout->addWidget(title, 0, Qt::AlignHCenter);
    panelLayout->addSpacing(16);
    panelLayout->addWidget(field, 0, Qt::AlignHCenter);
    panelLayout->addWidget(errorLabel, 0, Qt::AlignHCenter);
    panelLayout->addStretch(1);

    SpriteButton* continueButton = menu.makePushButton("Continue", this);
    continueButton->setFixedSize(buttonSize);

    QLabel* continueText = new QLabel("Continue", continueButton);
    continueText->setGeometry(continueButton->rect());
    continueText->setAlignment(Qt::AlignCenter);
    continueText->setStyleSheet(QString("font: bold 34px; color: %1;").arg(TEXT_COLOR));

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addStretch(1);
    layout->addWidget(logo, 0, Qt::AlignHCenter);
    layout->addStretch(1);
    layout->addWidget(panel, 0, Qt::AlignHCenter);
    layout->addSpacing(30);
    layout->addWidget(continueButton, 0, Qt::AlignHCenter);
    layout->addStretch(2);

    connect(input, &QLineEdit::returnPressed, this, &UsernameScreen::confirm);
    connect(input, &QLineEdit::textChanged, this, [this]() { errorLabel->clear(); });
    connect(continueButton, &QAbstractButton::clicked, this, &UsernameScreen::confirm);
}

void UsernameScreen::confirm() {
    const QString username = input->text().trimmed();
    if (!username.isEmpty()) {
        emit usernameConfirmed(username);
    }
}

void UsernameScreen::showError(const QString& message) {
    errorLabel->setText(message);
}
