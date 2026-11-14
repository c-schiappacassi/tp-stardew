#ifndef FARMERSPRITES_H
#define FARMERSPRITES_H

#include <QColor>
#include <QPixmap>
#include <vector>

class FarmerSprites {
public:
    FarmerSprites();

    std::vector<int> hairModels() const;
    std::vector<int> shirtModels() const;
    std::vector<int> pantsModels() const;
    QPixmap drawCharacter(int hairModel, const QColor& hairColor, int shirtModel, int pantsModel, const QColor& pantsColor) const;

private:
    QPixmap body;
    QPixmap arms;
    QPixmap hairSheet;
    QPixmap shirtSheet;
    QPixmap pantsSheet;
};

#endif  // FARMERSPRITES_H
