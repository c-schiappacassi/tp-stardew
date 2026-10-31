#include "usernamescreen.h"

#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QTransform>
#include <QVBoxLayout>

#include "spritebutton.h"

namespace {
const QRect logoRect(0, 0, 400, 187);
const QRect panelRect(1200, 51, 175, 201);
const QSize logoSize(960, 449);
const QSize panelSize(768, 304);
const QSize inputSize(480, 120);
const QSize buttonSize(320, 80);
const QString textColor = "#4a2a0a";
}

UsernameScreen::UsernameScreen(QWidget* parent): QWidget(parent), input(new QLineEdit(this)) {
    const QPixmap sheet(QString(ASSETS_DIR) + "/Sprites/Menus/Title Screen.png");
    QLabel* logo = new QLabel(this);
    logo->setPixmap(sheet.copy(logoRect));
    logo->setScaledContents(true);
    logo->setFixedSize(logoSize);


    const QPixmap frame = QPixmap(QString(ASSETS_DIR) + "/Sprites/Menus/FQMenu.png")
        .copy(panelRect)
        .transformed(QTransform()
        .rotate(90));

    QLabel* panel = new QLabel(this);
    panel->setPixmap(frame);
    panel->setScaledContents(true);
    panel->setFixedSize(panelSize);

    QLabel* title = new QLabel("Enter username", panel);
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

    const QPixmap button(QString(ASSETS_DIR) + "/Sprites/Menus/TextBox.png");
    SpriteButton* continueButton = new SpriteButton(button, button, this);
    continueButton->setFixedSize(buttonSize);

    QLabel* continueText = new QLabel("Continue", continueButton);
    continueText->setGeometry(continueButton->rect());
    continueText->setAlignment(Qt::AlignCenter);
    continueText->setStyleSheet(QString("font: bold 34px; color: %1;").arg(textColor));

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addStretch(1);
    layout->addWidget(logo, 0, Qt::AlignHCenter);
    layout->addStretch(1);
    layout->addWidget(panel, 0, Qt::AlignHCenter);
    layout->addSpacing(30);
    layout->addWidget(continueButton, 0, Qt::AlignHCenter);
    layout->addStretch(2);

    connect(input, &QLineEdit::returnPressed, this, &UsernameScreen::confirm);
    connect(continueButton, &QAbstractButton::clicked, this, &UsernameScreen::confirm);
}


void UsernameScreen::confirm() {
    const QString username = input->text().trimmed();
    if (!username.isEmpty()) {
        emit usernameConfirmed(username);
    }
}

