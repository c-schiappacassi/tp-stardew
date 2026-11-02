#include "newgamescreen.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QPainter>
#include <QSlider>
#include <QVBoxLayout>
#include <algorithm>

#include "spritebutton.h"

namespace {
const QRect panelRect(0, 0, 440, 260);
const QRect fieldRect(0, 620, 440, 100);
const QRect characterRect(40, 60, 80, 160);
const QRect leftArrowRect(352, 495, 13, 13);
const QRect rightArrowRect(365, 495, 14, 13);
const QRect okRect(128, 256, 64, 64);
const QRect backNormalRect(296, 252, 66, 28);
const QRect backHoverRect(296, 280, 66, 26);
const QSize panelSize(900, 840);
const QSize portraitSize(160, 240);
const QSize fieldSize(400, 90);
const QSize arrowSize(56, 52);
const QSize okSize(100, 100);
const QSize backSize(200, 85);

constexpr int labelWidth = 110;
constexpr int counterWidth = 130;
constexpr int firstBackHair = 40;
constexpr int lastBackHair = 47;

const QString textColor = "#4a2a0a";
const QString errorColor = "#a01010";

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

constexpr int sliderMax = 1000;
const QList<QColor> colorStops = {
        QColor(0, 0, 0),      QColor(60, 35, 15),  QColor(110, 70, 30),  QColor(170, 120, 60), QColor(230, 200, 120),
        QColor(230, 120, 30), QColor(200, 30, 30), QColor(230, 100, 160), QColor(130, 50, 170), QColor(40, 80, 210),
        QColor(40, 190, 210), QColor(50, 170, 60), QColor(255, 255, 255)};
constexpr int defaultHairColor = sliderMax * 2 / 12;
constexpr int defaultPantsColor = sliderMax * 9 / 12;

QColor colorAt(int value) {
    const double position = static_cast<double>(value) / sliderMax * (colorStops.size() - 1);
    const int i = qMin(static_cast<int>(position), static_cast<int>(colorStops.size()) - 2);
    const double t = position - i;
    const QColor& a = colorStops[i];
    const QColor& b = colorStops[i + 1];
    return QColor(a.red() + (b.red() - a.red()) * t, a.green() + (b.green() - a.green()) * t,
            a.blue() + (b.blue() - a.blue()) * t);
}

QString sliderStyle() {
    QString stops;
    for (int i = 0; i < colorStops.size(); ++i) {
        stops += QString("stop:%1 %2, ").arg(static_cast<double>(i) / (colorStops.size() - 1)).arg(colorStops[i].name());
    }
    stops.chop(2);
    return QString("QSlider::groove:horizontal { height: 6px; border: 3px solid #64320a;"
           " background: qlineargradient(x1:0, y1:0, x2:1, y2:0, %1); }"
           "QSlider::handle:horizontal { width: 16px; margin: -8px 0; background: #ffe6aa;"
           " border: 3px solid #64320a; }")
            .arg(stops);
}

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

Color toColor(const QColor& color) {
    return Color{static_cast<uint8_t>(color.red()), static_cast<uint8_t>(color.green()),
            static_cast<uint8_t>(color.blue())};
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

QLabel* makeLabel(const QString& text, QWidget* parent) {
    QLabel* label = new QLabel(text, parent);
    label->setStyleSheet(QString("font: bold 30px; color: %1;").arg(textColor));
    return label;
}

QLabel* makeField(QLineEdit* input, const QPixmap& frame, QWidget* parent) {
    input->setMaxLength(16);
    input->setStyleSheet(QString("QLineEdit { background: transparent; border: none; font: bold 32px; color: %1; }")
        .arg(textColor));

    QLabel* field = new QLabel(parent);
    field->setPixmap(frame);
    field->setScaledContents(true);
    field->setFixedSize(fieldSize);
    QHBoxLayout* fieldLayout = new QHBoxLayout(field);
    fieldLayout->setContentsMargins(20, 0, 20, 0);
    fieldLayout->addWidget(input);
    return field;
}

SpriteButton* makeArrowButton(const QPixmap& arrow, QWidget* parent) {
    SpriteButton* button = new SpriteButton(arrow, arrow, parent);
    button->setFixedSize(arrowSize);
    return button;
}
}

NewGameScreen::NewGameScreen(QWidget* parent): QWidget(parent) {
    QLabel* panel = new QLabel(this);
    const QPixmap frames(QString(ASSETS_DIR) + "/Sprites/Menus/Frames.png");
    panel->setPixmap(frames.copy(panelRect));
    panel->setScaledContents(true);
    panel->setFixedSize(panelSize);

    background = QPixmap(QString(ASSETS_DIR) + "/Sprites/Menus/DayBG.png");
    const QPixmap farmerSheet(QString(ASSETS_DIR) + "/Sprites/Characters/Farmer Base.png");
    body = farmerSheet.copy(bodyRect);
    arms = farmerSheet.copy(armsRect);
    hairSheet = QPixmap(QString(ASSETS_DIR) + "/Sprites/Characters/Hairstyles.png");
    shirtSheet = QPixmap(QString(ASSETS_DIR) + "/Sprites/Characters/Shirts.png");
    pantsSheet = QPixmap(QString(ASSETS_DIR) + "/Sprites/Characters/Pants.png");
    hairModels = validModels(hairSheet, hairLayout);
    hairModels.erase(std::remove_if(hairModels.begin(), hairModels.end(),
            [](int model) { return model >= firstBackHair && model <= lastBackHair; }),
            hairModels.end());
    shirtModels = validModels(shirtSheet, shirtLayout);
    pantsModels = validModels(pantsSheet, pantsLayout);

    nameInput = new QLineEdit(panel);
    farmNameInput = new QLineEdit(panel);
    errorLabel = new QLabel(panel);
    errorLabel->setStyleSheet(QString("font: bold 26px; color: %1;").arg(errorColor));

    portrait = new QLabel(panel);
    portrait->setFixedSize(portraitSize);

    QGridLayout* fieldsLayout = new QGridLayout();
    fieldsLayout->setHorizontalSpacing(10);
    fieldsLayout->setVerticalSpacing(40);
    QLabel* nameLabel = makeLabel("Name", panel);
    nameLabel->setFixedWidth(labelWidth);
    QLabel* farmLabel = makeLabel("Farm\nname", panel);
    farmLabel->setFixedWidth(labelWidth);
    fieldsLayout->addWidget(nameLabel, 0, 0);
    fieldsLayout->addWidget(makeField(nameInput, frames.copy(fieldRect), panel), 0, 1);
    fieldsLayout->addWidget(farmLabel, 1, 0);
    fieldsLayout->addWidget(makeField(farmNameInput, frames.copy(fieldRect), panel), 1, 1);
    fieldsLayout->addWidget(errorLabel, 2, 1);

    QVBoxLayout* rightColumn = new QVBoxLayout();
    rightColumn->addLayout(fieldsLayout);
    rightColumn->addStretch(1);

    QHBoxLayout* topRow = new QHBoxLayout();
    topRow->setSpacing(40);
    topRow->addWidget(portrait, 0, Qt::AlignTop);
    topRow->addLayout(rightColumn);
    topRow->addStretch(1);

    QGridLayout* rowsLayout = new QGridLayout();
    rowsLayout->setHorizontalSpacing(10);
    rowsLayout->setVerticalSpacing(30);
    hairColorSlider = addSelectorRow(rowsLayout, 0, "Hair", hairModels, hairIndex, true);
    addSelectorRow(rowsLayout, 1, "Shirt", shirtModels, shirtIndex, false);
    pantsColorSlider = addSelectorRow(rowsLayout, 2, "Pants", pantsModels, pantsIndex, true);
    hairColorSlider->setValue(defaultHairColor);
    pantsColorSlider->setValue(defaultPantsColor);
    connect(hairColorSlider, &QSlider::valueChanged, this, &NewGameScreen::updatePreview);
    connect(pantsColorSlider, &QSlider::valueChanged, this, &NewGameScreen::updatePreview);

    const QPixmap cursors(QString(ASSETS_DIR) + "/Sprites/Menus/Cursors.png");
    SpriteButton* okButton = new SpriteButton(cursors.copy(okRect), cursors.copy(okRect), panel);
    okButton->setFixedSize(okSize);

    QVBoxLayout* panelLayout = new QVBoxLayout(panel);
    panelLayout->setContentsMargins(80, 80, 80, 80);
    panelLayout->addLayout(topRow);
    panelLayout->addSpacing(50);
    panelLayout->addLayout(rowsLayout);
    panelLayout->addStretch(1);
    panelLayout->addWidget(okButton, 0, Qt::AlignRight);

    const QPixmap titleSheet(QString(ASSETS_DIR) + "/Sprites/Menus/Title Screen.png");
    SpriteButton* backButton = new SpriteButton(titleSheet.copy(backNormalRect), titleSheet.copy(backHoverRect), this);
    backButton->setFixedSize(backSize);

    QGridLayout* layout = new QGridLayout(this);
    layout->addWidget(panel, 0, 0, Qt::AlignCenter);
    layout->addWidget(backButton, 0, 0, Qt::AlignRight | Qt::AlignBottom);

    connect(backButton, &QAbstractButton::clicked, this, &NewGameScreen::backRequested);
    connect(okButton, &QAbstractButton::clicked, this, &NewGameScreen::confirm);
    connect(nameInput, &QLineEdit::textChanged, this, [this]() { errorLabel->clear(); });
    connect(farmNameInput, &QLineEdit::textChanged, this, [this]() { errorLabel->clear(); });

    updatePreview();
}

QSlider* NewGameScreen::addSelectorRow(QGridLayout* layout, int row, const QString& title, const std::vector<int>& models, int& index, bool withColor) {
    const QPixmap cursors(QString(ASSETS_DIR) + "/Sprites/Menus/Cursors.png");
    SpriteButton* leftButton = makeArrowButton(cursors.copy(leftArrowRect), this);
    SpriteButton* rightButton = makeArrowButton(cursors.copy(rightArrowRect), this);

    QLabel* nameLabel = makeLabel(title, this);
    nameLabel->setFixedWidth(labelWidth);
    QLabel* counter = makeLabel("", this);
    counter->setFixedWidth(counterWidth);
    counter->setAlignment(Qt::AlignCenter);

    const int count = static_cast<int>(models.size());
    auto refreshCounter = [counter, count, &index]() {
        counter->setText(QString("%1/%2").arg(index + 1).arg(count));
    };
    refreshCounter();

    connect(leftButton, &QAbstractButton::clicked, this, [this, refreshCounter, count, &index]() {
        index = (index + count - 1) % count;
        refreshCounter();
        updatePreview();
    });
    connect(rightButton, &QAbstractButton::clicked, this, [this, refreshCounter, count, &index]() {
        index = (index + 1) % count;
        refreshCounter();
        updatePreview();
    });

    layout->addWidget(nameLabel, row, 0);
    layout->addWidget(leftButton, row, 1);
    layout->addWidget(counter, row, 2);
    layout->addWidget(rightButton, row, 3);

    if (!withColor) {
        return nullptr;
    }
    QSlider* slider = new QSlider(Qt::Horizontal, this);
    slider->setRange(0, sliderMax);
    slider->setFixedHeight(50);
    slider->setStyleSheet(sliderStyle());
    layout->addWidget(slider, row, 4);
    return slider;
}

void NewGameScreen::updatePreview() {
    QPixmap character(bodyRect.size());
    character.fill(Qt::transparent);
    {
        QPainter painter(&character);
        painter.drawPixmap(0, 0, body);
        painter.drawPixmap(0, 0, tinted(pantsSheet.copy(pantsLayout.cell(pantsModels[pantsIndex])), colorAt(pantsColorSlider->value())));
        painter.drawPixmap(shirtOffset, shirtSheet.copy(shirtLayout.cell(shirtModels[shirtIndex])));
        painter.drawPixmap(hairOffset, tinted(hairSheet.copy(hairLayout.cell(hairModels[hairIndex])), colorAt(hairColorSlider->value())));
        painter.drawPixmap(0, 0, arms);
    }

    QPixmap canvas(portraitSize);
    {
        QPainter painter(&canvas);
        painter.drawPixmap(canvas.rect(), background);
        painter.drawPixmap(characterRect, character);
    }
    portrait->setPixmap(canvas);
}

void NewGameScreen::confirm() {
    if (!characterName().isEmpty() && !farmName().isEmpty()) {
        emit confirmed();
    }
}

void NewGameScreen::showError(const QString& message) {
    errorLabel->setText(message);
}

QString NewGameScreen::characterName() const {
    return nameInput->text().trimmed();
}

QString NewGameScreen::farmName() const {
    return farmNameInput->text().trimmed();
}

uint16_t NewGameScreen::hairModel() const {
    return hairModels[hairIndex];
}

uint16_t NewGameScreen::shirtModel() const {
    return shirtModels[shirtIndex];
}

uint16_t NewGameScreen::pantsModel() const {
    return pantsModels[pantsIndex];
}

Color NewGameScreen::hairColor() const {
    return toColor(colorAt(hairColorSlider->value()));
}

Color NewGameScreen::pantsColor() const {
    return toColor(colorAt(pantsColorSlider->value()));
}