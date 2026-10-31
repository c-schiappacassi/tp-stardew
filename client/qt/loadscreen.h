#ifndef LOADSCREEN_H
#define LOADSCREEN_H

#include <QWidget>

class LoadScreen : public QWidget {
    Q_OBJECT

public:
    explicit LoadScreen(QWidget* parent = nullptr);

signals:
    void backRequested();
};

#endif  // LOADSCREEN_H