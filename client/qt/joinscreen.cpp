#include "joinscreen.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QTransform>
#include <QVBoxLayout>

#include "spritebutton.h"

namespace {
const QSize panelSize(700, 380);
const QSize inputSize(480, 120);
const QSize continueButtonSize(320, 80);
const QRect backNormalRect(296, 252, 66, 28);
const QRect backHoverRect(296, 280, 66, 26);
const QSize backSize(200, 85);

const QString textColor = "#4a2a0a";

const QRect panelRect(1200, 51, 175, 201);

SpriteButton* makeButton(const QString& text, QWidget* parent) {
    const QPixmap pixmap(QString(ASSETS_DIR) + "/Sprites/Menus/TextBox.png");
    SpriteButton* button = new SpriteButton(pixmap, pixmap, parent);
    button->setFixedSize(continueButtonSize);

    QLabel* label = new QLabel(text, button);
    label->setGeometry(button->rect());
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet(QString("font: bold 34px; color: %1;").arg(textColor));
    return button;
}
}

JoinScreen::JoinScreen(QWidget* parent): QWidget(parent), input(new QLineEdit(this)) {
    const QPixmap frame = QPixmap(QString(ASSETS_DIR) + "/Sprites/Menus/FQMenu.png")
            .copy(panelRect)
            .transformed(QTransform()
            .rotate(90));

    QLabel* panel = new QLabel(this);
    panel->setPixmap(frame);
    panel->setScaledContents(true);
    panel->setFixedSize(panelSize);

    QLabel* title = new QLabel("Enter invite code", panel);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(QString("font: bold 40px; color: %1;").arg(textColor));

    input->setFixedSize(inputSize);
    input->setAlignment(Qt::AlignCenter);
    input->setMaxLength(16);
    input->setStyleSheet(QString("QLineEdit { border-image: url(\"%1TextBox.png\") 0 0 0 0 stretch stretch;"
             " font: bold 38px; color: %2; padding: 0 20px; }")
             .arg(QString(ASSETS_DIR) + "/Sprites/Menus/", textColor));

    QVBoxLayout* panelLayout = new QVBoxLayout(panel);
    panelLayout->addStretch(1);
    panelLayout->addWidget(title, 0, Qt::AlignHCenter);
    panelLayout->addSpacing(16);
    panelLayout->addWidget(input, 0, Qt::AlignHCenter);
    panelLayout->addStretch(1);

    const QPixmap titleSheet(QString(ASSETS_DIR) + "/Sprites/Menus/Title Screen.png");
    SpriteButton* backButton = new SpriteButton(titleSheet.copy(backNormalRect), titleSheet.copy(backHoverRect), this);
    backButton->setFixedSize(backSize);
    SpriteButton* joinButton = makeButton("Join", this);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addStretch(2);
    layout->addWidget(panel, 0, Qt::AlignHCenter);
    layout->addSpacing(30);
    layout->addWidget(joinButton, 0, Qt::AlignHCenter);
    layout->addStretch(2);
    layout->addWidget(backButton, 0, Qt::AlignRight);

    connect(input, &QLineEdit::returnPressed, this, &JoinScreen::confirm);
    connect(joinButton, &QAbstractButton::clicked, this, &JoinScreen::confirm);
    connect(backButton, &QAbstractButton::clicked, this, &JoinScreen::backRequested);
}

void JoinScreen::confirm() {
    const QString inviteCode = input->text().trimmed();
    if (!inviteCode.isEmpty()) {
        emit joinRequested(inviteCode);
    }
}