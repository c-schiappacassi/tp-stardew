#ifndef NEWGAMESCREEN_H
#define NEWGAMESCREEN_H

#include <QPixmap>
#include <QWidget>
#include <vector>

#include "dtos.h"

class QGridLayout;
class QLabel;
class QLineEdit;
class QSlider;

class NewGameScreen : public QWidget {
    Q_OBJECT

public:
    explicit NewGameScreen(QWidget* parent = nullptr);
    void showError(const QString& message);

    QString characterName() const;
    QString farmName() const;
    uint16_t hairModel() const;
    uint16_t shirtModel() const;
    uint16_t pantsModel() const;
    Color hairColor() const;
    Color pantsColor() const;

signals:
    void backRequested();
    void confirmed();

private:
    QSlider* addSelectorRow(QGridLayout* layout, int row, const QString& title, const std::vector<int>& models, int& index, bool withColor);
    void updatePreview();
    void confirm();

    QLineEdit* nameInput;
    QLineEdit* farmNameInput;
    QLabel* portrait;
    QLabel* errorLabel;

    QPixmap background;
    QPixmap body;
    QPixmap arms;
    QPixmap hairSheet;
    QPixmap shirtSheet;
    QPixmap pantsSheet;

    std::vector<int> hairModels;
    std::vector<int> shirtModels;
    std::vector<int> pantsModels;
    int hairIndex = 0;
    int shirtIndex = 0;
    int pantsIndex = 0;

    QSlider* hairColorSlider = nullptr;
    QSlider* pantsColorSlider = nullptr;
};

#endif  // NEWGAMESCREEN_H