#ifndef EDITOR_CONSTANTS_H
#define EDITOR_CONSTANTS_H

#include <QString>

const QString MENUS_PATH = QString(ASSETS_DIR) + "/sprites/menus/";
const QString EDITOR_PATH = QString(ASSETS_DIR) + "/sprites/editor/";
const QString SPRITES_PATH = QString(ASSETS_DIR) + "/sprites/";
const QString CUSTOM_MAPS_PATH = QString(MAPS_DIR) + "/custom/";

const QString TEXT_COLOR = "#4a2a0a";

// estilo de los bordes de las tabs, la barra lateral, la grilla, mensajes
const QString TABS_STYLE = R"(
    QTabWidget::pane { border: 4px solid #55281c; background: #f6d7a0; }
    QTabBar::tab { background: #d4a463; color: #4a2a0a; border: 4px solid #55281c; border-bottom: none;
                   padding: 8px 16px; margin-right: 2px; font-weight: bold; font-size: 14px; }
    QTabBar::tab:selected { background: #f6d7a0; }
    QTabBar::tab:bottom { border-top: none; }
    QTabWidget QTabWidget::pane { border: none; border-bottom: 4px solid #55281c; }
    QTabBar::tab:hover:!selected { background: #dcb072; }

    QListWidget { background: #f6d7a0; border: 4px solid #81411c; color: #4a2a0a; font-size: 14px; outline: 0; }
    QListWidget::item { padding: 4px; }
    QListWidget::item:disabled { color: #81411c; font-weight: bold; }
    QListWidget::item:selected { background: #e9c58b; color: #4a2a0a; border: 2px solid #a05720; }
    QListWidget::item:hover:!selected { background: #efcc94; }

    QGraphicsView { background: #e9c58b; border: 4px solid #81411c; }
    QLabel { color: #4a2a0a; font-size: 14px; }

    QScrollBar:vertical { background: #e9c58b; width: 14px; }
    QScrollBar::handle { background: #a05720; border: 2px solid #55281c; }
    QScrollBar::add-line, QScrollBar::sub-line { height: 0; width: 0; }
)";

// boton Save
const QString BUTTON_STYLE = R"(
    QPushButton { background: #d4a463; color: #4a2a0a; border: 4px solid #55281c; padding: 10px 28px;
                  font-weight: bold; font-size: 16px; }
    QPushButton:hover { background: #dcb072; }
    QPushButton:pressed { background: #f6d7a0; }
)";

#endif  // EDITOR_CONSTANTS_H
