#ifndef LOADSCREEN_H
#define LOADSCREEN_H

#include <QWidget>
#include <vector>

#include "dtos.h"

class QLabel;
class QVBoxLayout;
class SpriteButton;

class LoadScreen : public QWidget {
    Q_OBJECT

public:
    explicit LoadScreen(QWidget* parent = nullptr);
    void setGames(const std::vector<GameInfoDTO>& games);

signals:
    void backRequested();
    void resumeRequested(const std::string& gameId);

private:
    SpriteButton* makeRow(const GameInfoDTO& gameInfo);
    void showRows();
    std::vector<GameInfoDTO> games;
    int firstRow = 0;
    QVBoxLayout* rowsLayout;
    QPixmap frames;
};

#endif  // LOADSCREEN_H