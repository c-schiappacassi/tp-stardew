#include "character_screen.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QPainter>
#include <QSlider>
#include <QVBoxLayout>

#include "sprite_button.h"
#include "menu_helpers.h"
#include "qt_constants.h"

namespace {
const QSize panelSize(900, 840);

const QList<QColor> colorStops = {
        QColor(0, 0, 0),      QColor(60, 35, 15),  QColor(110, 70, 30),  QColor(170, 120, 60), QColor(230, 200, 120),
        QColor(230, 120, 30), QColor(200, 30, 30), QColor(230, 100, 160), QColor(130, 50, 170), QColor(40, 80, 210),
        QColor(40, 190, 210), QColor(50, 170, 60), QColor(255, 255, 255)};
constexpr int defaultHairColor = SLIDER_MAX * 2 / 12;
constexpr int defaultPantsColor = SLIDER_MAX * 9 / 12;

QColor colorAt(int value) {
    const double position = static_cast<double>(value) / SLIDER_MAX * (colorStops.size() - 1);
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

Color toColor(const QColor& color) {
    return Color{static_cast<uint8_t>(color.red()), static_cast<uint8_t>(color.green()),
            static_cast<uint8_t>(color.blue())};
}

SpriteButton* makeArrowButton(const QPixmap& arrow, QWidget* parent) {
    SpriteButton* button = new SpriteButton(arrow, arrow, parent);
    button->setFixedSize(ARROW_SIZE);
    return button;
}
}

CharacterScreen::CharacterScreen(QWidget* parent): QWidget(parent) {
    panel = menu.makePanel(PANEL_RECT, panelSize, this);

    background = QPixmap(MENUS_PATH + "day_bg.png");
    hairModels = sprites.hairModels();
    shirtModels = sprites.shirtModels();
    pantsModels = sprites.pantsModels();

    nameInput = new QLineEdit(panel);
    nameInput->setMaxLength(16);
    nameInput->setStyleSheet(QString("QLineEdit { background: transparent; border: none; font: bold 32px; color: %1; }")
        .arg(TEXT_COLOR));
    errorLabel = new QLabel(panel);
    errorLabel->setStyleSheet(QString("font: bold 26px; color: %1;").arg(ERROR_COLOR));

    portrait = new QLabel(panel);
    portrait->setFixedSize(PORTRAIT_SIZE);
}

void CharacterScreen::buildLayout(QVBoxLayout* rightColumn, QWidget* sidePanel) {
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
    connect(hairColorSlider, &QSlider::valueChanged, this, &CharacterScreen::updatePreview);
    connect(pantsColorSlider, &QSlider::valueChanged, this, &CharacterScreen::updatePreview);

    const QPixmap cursors(MENUS_PATH + "cursors.png");
    SpriteButton* okButton = new SpriteButton(cursors.copy(OK_RECT), cursors.copy(OK_RECT), panel);
    okButton->setFixedSize(OK_SIZE);

    QVBoxLayout* panelLayout = new QVBoxLayout(panel);
    panelLayout->setContentsMargins(80, 80, 80, 80);
    panelLayout->addLayout(topRow);
    panelLayout->addSpacing(50);
    panelLayout->addLayout(rowsLayout);
    panelLayout->addStretch(1);
    panelLayout->addWidget(okButton, 0, Qt::AlignRight);

    SpriteButton* backButton = menu.makeBackButton(this);


    QHBoxLayout* center = new QHBoxLayout();
    center->setSpacing(40);
    center->addStretch(1);
    center->addWidget(panel);
    if (sidePanel != nullptr) {
        center->addWidget(sidePanel);
    }
    center->addStretch(1);

    QGridLayout* layout = new QGridLayout(this);
    layout->addLayout(center, 0, 0);
    layout->addWidget(backButton, 0, 0, Qt::AlignRight | Qt::AlignBottom);

    connect(backButton, &QAbstractButton::clicked, this, &CharacterScreen::backRequested);
    connect(okButton, &QAbstractButton::clicked, this, &CharacterScreen::confirm);
    connect(nameInput, &QLineEdit::textChanged, this, [this]() { errorLabel->clear(); });

    updatePreview();
}

QSlider* CharacterScreen::addSelectorRow(QGridLayout* layout, int row, const QString& title, const std::vector<int>& models, int& index, bool withColor) {
    const QPixmap cursors(MENUS_PATH + "cursors.png");
    SpriteButton* leftButton = makeArrowButton(cursors.copy(LEFT_ARROW_RECT), this);
    SpriteButton* rightButton = makeArrowButton(cursors.copy(RIGHT_ARROW_RECT), this);

    QLabel* nameLabel = makeLabel(title, this);
    nameLabel->setFixedWidth(LABEL_WIDTH);
    QLabel* counter = makeLabel("", this);
    counter->setFixedWidth(COUNTER_WIDTH);
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
    slider->setRange(0, SLIDER_MAX);
    slider->setFixedHeight(50);
    slider->setStyleSheet(sliderStyle());
    layout->addWidget(slider, row, 4);
    return slider;
}

void CharacterScreen::updatePreview() {
    QPixmap character = sprites.drawCharacter(hairModels[hairIndex], colorAt(hairColorSlider->value()),
            shirtModels[shirtIndex], pantsModels[pantsIndex], colorAt(pantsColorSlider->value()));

    QPixmap canvas(PORTRAIT_SIZE);
    canvas.fill(Qt::transparent);
    {
        QPainter painter(&canvas);
        painter.drawPixmap(canvas.rect(), background);
        painter.drawPixmap(CHARACTER_RECT, character);
    }
    portrait->setPixmap(canvas);
}

QLabel* CharacterScreen::makeLabel(const QString& text, QWidget* parent) {
    QLabel* label = new QLabel(text, parent);
    label->setStyleSheet(QString("font: bold 30px; color: %1;").arg(TEXT_COLOR));
    return label;
}

void CharacterScreen::showError(const QString& message) {
    errorLabel->setText(message);
}

QString CharacterScreen::characterName() const {
    return nameInput->text().trimmed();
}

uint16_t CharacterScreen::hairModel() const {
    return hairModels[hairIndex];
}

uint16_t CharacterScreen::shirtModel() const {
    return shirtModels[shirtIndex];
}

uint16_t CharacterScreen::pantsModel() const {
    return pantsModels[pantsIndex];
}

Color CharacterScreen::hairColor() const {
    return toColor(colorAt(hairColorSlider->value()));
}

Color CharacterScreen::pantsColor() const {
    return toColor(colorAt(pantsColorSlider->value()));
}
