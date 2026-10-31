#ifndef NEWGAMESCREEN_H
#define NEWGAMESCREEN_H

#include <QPixmap>
#include <QWidget>
#include <vector>

class QGridLayout;
class QLabel;
class QLineEdit;
class QSlider;

class NewGameScreen : public QWidget {
    Q_OBJECT

public:
    explicit NewGameScreen(QWidget* parent = nullptr);

signals:
    void backRequested();

private:
    QSlider* addSelectorRow(QGridLayout* layout, int row, const QString& title, const std::vector<int>& models, int& index, bool withColor);
    void updatePreview();

    QLineEdit* nameInput;
    QLineEdit* farmNameInput;
    QLabel* portrait;

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