#include "menu_helpers.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>

#include "sprite_button.h"
#include "qt_constants.h"

namespace {
const QRect logoRect(0, 0, 400, 187);
const QRect buttonRect(480, 760, 300, 80);
const QRect buttonHoverRect(800, 760, 300, 80);
const QRect backNormalRect(296, 252, 66, 28);
const QRect backHoverRect(296, 280, 66, 26);
const QSize logoSize(960, 449);
const QSize buttonSize(320, 80);
const QSize backSize(200, 85);
}

MenuHelpers::MenuHelpers() : frames(MENUS_PATH + "frames.png"), titleSheet(MENUS_PATH + "title_screen.png") {}

QLabel* MenuHelpers::makeLogo(QWidget* parent) const {
    QLabel* logo = new QLabel(parent);
    logo->setPixmap(titleSheet.copy(logoRect));
    logo->setScaledContents(true);
    logo->setFixedSize(logoSize);
    return logo;
}

QLabel* MenuHelpers::makePanel(const QRect& rect, const QSize& size, QWidget* parent) const {
    QLabel* panel = new QLabel(parent);
    panel->setPixmap(frames.copy(rect));
    panel->setScaledContents(true);
    panel->setFixedSize(size);
    return panel;
}

QLabel* MenuHelpers::makeInputFrame(QLineEdit* input, const QRect& rect, const QSize& size, QWidget* parent) const {
    QLabel* field = new QLabel(parent);
    field->setPixmap(frames.copy(rect));
    field->setScaledContents(true);
    field->setFixedSize(size);
    QHBoxLayout* fieldLayout = new QHBoxLayout(field);
    fieldLayout->setContentsMargins(20, 0, 20, 0);
    fieldLayout->addWidget(input);
    return field;
}

SpriteButton* MenuHelpers::makePushButton(const QString& text, QWidget* parent) const {
    SpriteButton* button = new SpriteButton(frames.copy(buttonRect), frames.copy(buttonHoverRect), parent);
    button->setFixedSize(buttonSize);

    QLabel* label = new QLabel(text, button);
    label->setGeometry(button->rect());
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet(QString("font: bold 34px; color: %1;").arg(TEXT_COLOR));
    return button;
}

SpriteButton* MenuHelpers::makeBackButton(QWidget* parent) const {
    SpriteButton* button = new SpriteButton(titleSheet.copy(backNormalRect), titleSheet.copy(backHoverRect), parent);
    button->setFixedSize(backSize);
    return button;
}