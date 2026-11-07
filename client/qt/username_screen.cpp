#include "username_screen.h"

#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "sprite_button.h"

namespace {
const QRect logoRect(0, 0, 400, 187);
const QRect panelRect(0, 300, 440, 260);
const QRect fieldRect(0, 620, 440, 100);
const QRect buttonRect(480, 760, 300, 80);
const QRect buttonHoverRect(800, 760, 300, 80);
const QSize logoSize(960, 449);
const QSize panelSize(768, 304);
const QSize fieldSize(480, 100);
const QSize buttonSize(320, 80);
const QSize errorSize(640, 80);
const QString textColor = "#4a2a0a";
const QString errorColor = "#a01010";
}

UsernameScreen::UsernameScreen(QWidget* parent): QWidget(parent), input(new QLineEdit(this)) {
    const QPixmap sheet(QString(ASSETS_DIR) + "/sprites/menus/title_screen.png");
    QLabel* logo = new QLabel(this);
    logo->setPixmap(sheet.copy(logoRect));
    logo->setScaledContents(true);
    logo->setFixedSize(logoSize);

    const QPixmap frames = QPixmap(QString(ASSETS_DIR) + "/sprites/menus/frames.png");

    QLabel* panel = new QLabel(this);
    panel->setPixmap(frames.copy(panelRect));
    panel->setScaledContents(true);
    panel->setFixedSize(panelSize);

    QLabel* title = new QLabel("Enter username", panel);
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

    SpriteButton* continueButton = new SpriteButton(frames.copy(buttonRect), frames.copy(buttonHoverRect), this);
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
