#ifndef JOINCHARACTERSCREEN_H
#define JOINCHARACTERSCREEN_H

#include "character_screen.h"

class JoinCharacterScreen : public CharacterScreen {
    Q_OBJECT

public:
    explicit JoinCharacterScreen(QWidget* parent = nullptr);

private:
    void confirm() override;
};

#endif  // JOINCHARACTERSCREEN_H
