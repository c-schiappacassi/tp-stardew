#pragma once

#include <QWidget>

class StartScreen : public QWidget {
    Q_OBJECT

public:
    explicit StartScreen(QWidget* parent = nullptr);

signals:
    void createMapRequested();
    void editMapRequested();
};
