#ifndef MENU_HELPERS_H
#define MENU_HELPERS_H

#include <QPixmap>
#include <QRect>
#include <QSize>
#include <QString>

class QLabel;
class QLineEdit;
class QWidget;
class SpriteButton;

class MenuHelpers {
public:
    MenuHelpers();

    QPixmap frame(const QRect& rect) const;
    QLabel* makeLogo(QWidget* parent) const;
    QLabel* makePanel(const QRect& rect, const QSize& size, QWidget* parent) const;
    QLabel* makeInputFrame(QLineEdit* input, const QRect& rect, const QSize& size, QWidget* parent) const;
    SpriteButton* makePushButton(const QString& text, QWidget* parent) const;
    SpriteButton* makeBackButton(QWidget* parent) const;

private:
    QPixmap frames;
    QPixmap titleSheet;
};


#endif  // MENU_HELPERS_H
