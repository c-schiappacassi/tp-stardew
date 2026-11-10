#include "create_map_screen.h"

#include <QLabel>
#include <QPushButton>

#include "editor_constants.h"

CreateMapScreen::CreateMapScreen(QWidget* parent): QWidget(parent) {
    QPushButton* backButton = new QPushButton("BACK", this);
    backButton->setFixedSize(200, 60);


    connect(backButton, &QPushButton::clicked, this, &CreateMapScreen::backRequested);
}
