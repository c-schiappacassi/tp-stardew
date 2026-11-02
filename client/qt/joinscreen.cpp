#include "joinscreen.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QVBoxLayout>

#include "spritebutton.h"

namespace {
const QRect panelRect(0, 300, 440, 260);
const QRect fieldRect(0, 620, 440, 100);
const QRect buttonRect(480, 760, 300, 80);
const QRect buttonHoverRect(800, 760, 300, 80);
const QRect backNormalRect(296, 252, 66, 28);
const QRect backHoverRect(296, 280, 66, 26);
const QSize panelSize(768, 380);
const QSize fieldSize(480, 120);
const QSize buttonSize(320, 80);
const QSize errorSize(640, 80);
const QSize backSize(200, 85);

const QString textColor = "#4a2a0a";
const QString errorColor = "#a01010";
}

JoinScreen::JoinScreen(QWidget* parent): QWidget(parent), input(new QLineEdit(this)) {
    const QPixmap frames(QString(ASSETS_DIR) + "/Sprites/Menus/Frames.png");

    QLabel* panel = new QLabel(this);
    panel->setPixmap(frames.copy(panelRect));
    panel->setScaledContents(true);
    panel->setFixedSize(panelSize);

    QLabel* title = new QLabel("Enter invite code", panel);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(QString("font: bold 40px; color: %1;").arg(textColor));

    input->setAlignment(Qt::AlignCenter);
    input->setMaxLength(16);
    input->setStyleSheet(QString("QLineEdit { background: transparent; border: none; font: bold 38px; color: %1; }")
            .arg(textColor));

    QLabel* field = new QLabel(panel);
    field->setPixmap(frames.copy(fieldRect));
    field->setScaledContents(true);
    field->setFixedSize(fieldSize);

    QHBoxLayout* fieldLayout = new QHBoxLayout(field);
    fieldLayout->setContentsMargins(20, 0, 20, 0);
    fieldLayout->addWidget(input);

    errorLabel = new QLabel(panel);
    errorLabel->setFixedSize(errorSize);
    errorLabel->setWordWrap(true);
    errorLabel->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    errorLabel->setStyleSheet(QString("font: bold 26px; color: %1;").arg(errorColor));

    QVBoxLayout* panelLayout = new QVBoxLayout(panel);
    panelLayout->addStretch(1);
    panelLayout->addWidget(title, 0, Qt::AlignHCenter);
    panelLayout->addSpacing(16);
    panelLayout->addWidget(field, 0, Qt::AlignHCenter);
    panelLayout->addWidget(errorLabel, 0, Qt::AlignHCenter);
    panelLayout->addStretch(1);

    SpriteButton* joinButton = new SpriteButton(frames.copy(buttonRect), frames.copy(buttonHoverRect), this);
    joinButton->setFixedSize(buttonSize);

    QLabel* joinText = new QLabel("Join", joinButton);
    joinText->setGeometry(joinButton->rect());
    joinText->setAlignment(Qt::AlignCenter);
    joinText->setStyleSheet(QString("font: bold 34px; color: %1;").arg(textColor));

    const QPixmap titleSheet(QString(ASSETS_DIR) + "/Sprites/Menus/Title Screen.png");
    SpriteButton* backButton = new SpriteButton(titleSheet.copy(backNormalRect), titleSheet.copy(backHoverRect), this);
    backButton->setFixedSize(backSize);

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
