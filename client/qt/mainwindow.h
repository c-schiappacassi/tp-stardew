#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <string>
#include <vector>
#include "dtos.h"

class NetworkManager;
class QStackedWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(NetworkManager& net, QWidget *parent = nullptr);

private:
    ServerStatus send(const CommandDTO& dto);

    QStackedWidget *stack;
    NetworkManager& net;
    std::string username;
    std::vector<GameInfoDTO> games;
};

#endif // MAINWINDOW_H
