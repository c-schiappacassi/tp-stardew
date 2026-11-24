#ifndef NEWGAMESCREEN_H
#define NEWGAMESCREEN_H

#include "character_screen.h"
#include <string>

class QLineEdit;
class MapSelector;

class NewGameScreen : public CharacterScreen {
    Q_OBJECT

public:
    explicit NewGameScreen(QWidget* parent = nullptr);

    QString farmName() const;
    std::string mapPath() const;

private:
    void confirm() override;

    QLineEdit* farmNameInput;
    MapSelector* mapSelector;
};

#endif  // NEWGAMESCREEN_H
