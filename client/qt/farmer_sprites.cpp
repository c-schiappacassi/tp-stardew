#include "farmer_sprites.h"

#include <QImage>
#include <QPainter>

#include "qt_constants.h"

namespace {
struct SheetLayout {
    int perRow;
    int stepX;
    int stepY;
    int cellWidth;
    int cellHeight;

    QRect cell(int model) const {
        return QRect((model % perRow) * stepX, (model / perRow) * stepY, cellWidth, cellHeight);
    }
};

const SheetLayout hairLayout{16, 16, 96, 16, 32};
const SheetLayout shirtLayout{16, 8, 32, 8, 8};
const SheetLayout pantsLayout{10, 192, 688, 16, 32};
const QRect bodyRect(0, 0, 16, 32);
const QRect armsRect(96, 0, 16, 32);
const QPoint shirtOffset(4, 15);
const QPoint hairOffset(0, 1);

QPixmap tinted(const QPixmap& sprite, const QColor& color) {
    QImage image = sprite.toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);
    QPainter painter(&image);
    painter.setCompositionMode(QPainter::CompositionMode_Multiply);
    painter.fillRect(image.rect(), color);
    painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    painter.drawPixmap(0, 0, sprite);
    painter.end();
    return QPixmap::fromImage(image);
}

// descarto los tipos de peloo vacios
std::vector<int> validModels(const QPixmap& sheet, const SheetLayout& layout) {
    const QImage image = sheet.toImage();
    std::vector<int> models;
    for (int model = 0; layout.cell(model).bottom() < image.height(); ++model) {
        const QImage cell = image.copy(layout.cell(model));
        bool empty = true;
        for (int y = 0; y < cell.height() && empty; ++y) {
            for (int x = 0; x < cell.width() && empty; ++x) {
                empty = qAlpha(cell.pixel(x, y)) == 0;
            }
        }
        if (!empty) {
            models.push_back(model);
        }
    }
    return models;
}
}

FarmerSprites::FarmerSprites() {
    const QPixmap farmerSheet(CHARACTERS_PATH + "farmer_base.png");
    body = farmerSheet.copy(bodyRect);
    arms = farmerSheet.copy(armsRect);
    hairSheet = QPixmap(CHARACTERS_PATH + "hairstyles.png");
    shirtSheet = QPixmap(CHARACTERS_PATH + "shirts.png");
    pantsSheet = QPixmap(CHARACTERS_PATH + "pants.png");
}

std::vector<int> FarmerSprites::hairModels() const {
    return validModels(hairSheet, hairLayout);
}

std::vector<int> FarmerSprites::shirtModels() const {
    return validModels(shirtSheet, shirtLayout);
}

std::vector<int> FarmerSprites::pantsModels() const {
    return validModels(pantsSheet, pantsLayout);
}

QPixmap FarmerSprites::drawCharacter(int hairModel, const QColor& hairColor, int shirtModel, int pantsModel, const QColor& pantsColor) const {
    QPixmap character(bodyRect.size());
    character.fill(Qt::transparent);
    {
        QPainter painter(&character);
        painter.drawPixmap(0, 0, body);
        painter.drawPixmap(0, 0, tinted(pantsSheet.copy(pantsLayout.cell(pantsModel)), pantsColor));
        painter.drawPixmap(shirtOffset, shirtSheet.copy(shirtLayout.cell(shirtModel)));
        painter.drawPixmap(hairOffset, tinted(hairSheet.copy(hairLayout.cell(hairModel)), hairColor));
        painter.drawPixmap(0, 0, arms);
    }
    return character;
}
