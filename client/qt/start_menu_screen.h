#ifndef STARTMENUSCREEN_H
#define STARTMENUSCREEN_H

#include <QWidget>

class StartMenuScreen : public QWidget {
    Q_OBJECT

public:
    explicit StartMenuScreen(QWidget* parent = nullptr);

signals:
    void newGameRequested();
    void loadGameRequested();
    void joinGameRequested();
    void exitRequested();
};

#endif // STARTMENUSCREEN_H
