#pragma once

#include <QRect>
#include <QSize>
#include <QString>

const QString MENUS_PATH = QString(ASSETS_DIR) + "/sprites/menus/";
const QString CHARACTERS_PATH = QString(ASSETS_DIR) + "/sprites/characters/";

const QString TEXT_COLOR = "#4a2a0a";
const QString LIGHTER_TEXT_COLOR = "#8a5a2a";
const QString ERROR_COLOR = "#a01010";

const QRect PANEL_RECT(0, 0, 440, 260);
const QRect POST_PANEL_RECT(0, 300, 440, 260);
const QRect FIELD_RECT(0, 620, 440, 100);
const QRect ROW_HOVER_RECT(0, 760, 440, 100);

const QRect CHARACTER_RECT(40, 60, 80, 160);
const QRect LEFT_ARROW_RECT(352, 495, 13, 13);
const QRect RIGHT_ARROW_RECT(365, 495, 14, 13);
const QRect OK_RECT(128, 256, 64, 64);
const QSize PORTRAIT_SIZE(160, 240);
const QSize ARROW_SIZE(56, 52);
const QSize OK_SIZE(100, 100);
const int LABEL_WIDTH = 110;
const int COUNTER_WIDTH = 130;
const int SLIDER_MAX = 1000;
