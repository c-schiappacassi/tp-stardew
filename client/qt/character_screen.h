#ifndef CHARACTERSCREEN_H
#define CHARACTERSCREEN_H

#include <QPixmap>
#include <QWidget>
#include <vector>

#include "dtos.h"
#include "farmer_sprites.h"
#include "menu_helpers.h"

class QGridLayout;
class QLabel;
class QLineEdit;
class QSlider;
class QVBoxLayout;

class CharacterScreen : public QWidget {
    Q_OBJECT

public:
    explicit CharacterScreen(QWidget* parent = nullptr);
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

protected:
    void buildLayout(QVBoxLayout* rightColumn, QWidget* sidePanel = nullptr);
    virtual void confirm() = 0;
    static QLabel* makeLabel(const QString& text, QWidget* parent);

    const MenuHelpers menu;
    QLabel* panel;
    QLineEdit* nameInput;
    QLabel* errorLabel;

private:
    QSlider* addSelectorRow(QGridLayout* layout, int row, const QString& title, const std::vector<int>& models, int& index, bool withColor);
    void updatePreview();

    QLabel* portrait;

    FarmerSprites sprites;
    QPixmap background;

    std::vector<int> hairModels;
    std::vector<int> shirtModels;
    std::vector<int> pantsModels;
    int hairIndex = 0;
    int shirtIndex = 0;
    int pantsIndex = 0;

    QSlider* hairColorSlider = nullptr;
    QSlider* pantsColorSlider = nullptr;
};

#endif  // CHARACTERSCREEN_H
