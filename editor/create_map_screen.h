#ifndef CREATE_MAP_SCREEN_H
#define CREATE_MAP_SCREEN_H

#include <QWidget>

class CreateMapScreen : public QWidget {
    Q_OBJECT

public:
    explicit CreateMapScreen(QWidget* parent = nullptr);

signals:
    void backRequested();
};

#endif  // CREATE_MAP_SCREEN_H
