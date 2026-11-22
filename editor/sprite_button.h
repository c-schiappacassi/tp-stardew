#ifndef SPRITE_BUTTON_H
#define SPRITE_BUTTON_H

#include <QAbstractButton>
#include <QPixmap>

class SpriteButton : public QAbstractButton {
    Q_OBJECT

public:
    SpriteButton(const QPixmap& normal, const QPixmap& hover, QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QPixmap normal;
    QPixmap hover;
};

#endif  // SPRITE_BUTTON_H
