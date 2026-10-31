#include "spritebutton.h"

#include <QPainter>

SpriteButton::SpriteButton(const QPixmap& normal, const QPixmap& hover, QWidget* parent): QAbstractButton(parent), normal(normal), hover(hover) {
    setAttribute(Qt::WA_Hover);
}

void SpriteButton::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.drawPixmap(rect(), underMouse() ? hover : normal);
}
