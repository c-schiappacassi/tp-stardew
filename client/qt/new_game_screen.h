#ifndef NEWGAMESCREEN_H
#define NEWGAMESCREEN_H

#include "character_screen.h"

class QLineEdit;

class NewGameScreen : public CharacterScreen {
    Q_OBJECT

public:
    explicit NewGameScreen(QWidget* parent = nullptr);

    QString farmName() const;

private:
    void confirm() override;

    QLineEdit* farmNameInput;
};

#endif  // NEWGAMESCREEN_H
