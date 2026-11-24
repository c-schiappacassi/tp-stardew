#ifndef MAPSELECTOR_H
#define MAPSELECTOR_H

#include <QString>
#include <QWidget>
#include <string>

class MenuHelpers;
class QButtonGroup;
class QLabel;

class MapSelector : public QWidget {
    Q_OBJECT

public:
    explicit MapSelector(const MenuHelpers& menu, QWidget* parent = nullptr);

    std::string mapPath() const;

private:
    void chooseCustomMap();

    QButtonGroup* defaults;
    QLabel* customLabel;
    QString customMap;
};

#endif  // MAPSELECTOR_H