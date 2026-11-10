#ifndef START_SCREEN_H
#define START_SCREEN_H

#include <QWidget>

class StartScreen : public QWidget {
    Q_OBJECT

public:
    explicit StartScreen(QWidget* parent = nullptr);

signals:
    void createMapRequested();
    void editMapRequested();
};

#endif  // START_SCREEN_H
