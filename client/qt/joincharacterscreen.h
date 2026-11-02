#ifndef JOINCHARACTERSCREEN_H
#define JOINCHARACTERSCREEN_H

#include <QPixmap>
#include <QWidget>
#include <vector>

#include "dtos.h"

class QGridLayout;
class QLabel;
class QLineEdit;
class QSlider;

class JoinCharacterScreen : public QWidget {
    Q_OBJECT

public:
    explicit JoinCharacterScreen(QWidget* parent = nullptr);
    void showError(const QString& message);

    QString characterName() const;
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

#endif  // JOINCHARACTERSCREEN_H