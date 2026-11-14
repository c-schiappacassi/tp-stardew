#include "join_screen.h"

#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>

#include "sprite_button.h"
#include "menu_helpers.h"
#include "qt_constants.h"

namespace {
const QSize panelSize(768, 380);
const QSize fieldSize(480, 120);
const QSize errorSize(640, 80);
}

JoinScreen::JoinScreen(QWidget* parent): QWidget(parent), input(new QLineEdit(this)) {
    const MenuHelpers menu;

    QLabel* panel = menu.makePanel(POST_PANEL_RECT, panelSize, this );

    QLabel* title = new QLabel("Enter invite code", panel);
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

    SpriteButton* joinButton = menu.makePushButton("Join", this);

    SpriteButton* backButton = menu.makeBackButton(this);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addStretch(2);
    layout->addWidget(panel, 0, Qt::AlignHCenter);
    layout->addSpacing(30);
    layout->addWidget(joinButton, 0, Qt::AlignHCenter);
    layout->addStretch(2);
    layout->addWidget(backButton, 0, Qt::AlignRight);

    connect(input, &QLineEdit::returnPressed, this, &JoinScreen::confirm);
    connect(input, &QLineEdit::textChanged, this, [this]() { errorLabel->clear(); });
    connect(joinButton, &QAbstractButton::clicked, this, &JoinScreen::confirm);
    connect(backButton, &QAbstractButton::clicked, this, &JoinScreen::backRequested);
}

void JoinScreen::confirm() {
    const QString inviteCode = input->text().trimmed();
    if (!inviteCode.isEmpty()) {
        emit joinRequested(inviteCode);
    }
}

void JoinScreen::showError(const QString& message) {
    errorLabel->setText(message);
}
