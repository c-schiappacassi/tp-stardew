#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <string>
#include <vector>
#include <optional>
#include "dtos.h"

class NetworkManager;
class QStackedWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(NetworkManager& net, QWidget *parent = nullptr);
    const std::optional<ServerStatus>& gameStatus() const { return entryStatus; }
    const std::string& userName() const { return username; }

private:
    ServerStatus send(const CommandDTO& dto);
    qtStatus sendQt(const CommandDTO& dto);
    void enterGame(const ServerStatus& status);

    QStackedWidget *stack;
    NetworkManager& net;
    std::string username;
    std::vector<GameInfoDTO> games;
    std::string joinCode;
    std::optional<ServerStatus> entryStatus;
};

#endif // MAINWINDOW_H
