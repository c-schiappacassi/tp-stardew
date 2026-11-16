#ifndef ZONE_CANVAS_H
#define ZONE_CANVAS_H

#include <vector>

#include <QGraphicsView>

#include "../model/zone_map.h"
#include "sprites.h"

class QGraphicsPixmapItem;
class QGraphicsRectItem;

class ZoneCanvas : public QGraphicsView {
    Q_OBJECT

public:
    ZoneCanvas(const ZoneMap& zone, const Sprites& sprites, QWidget* parent = nullptr);
    void refreshTile(int x, int y);
    void refreshElements();

    void showGhost(ElementId element, int x, int y, bool canPlace);
    void clearGhost();

signals:
    void tileClicked(int x, int y);
    void tileDragged(int x, int y);
    void tileHovered(int x, int y);
    void hoverLeft();
    void rightClicked();

protected:
    void drawForeground(QPainter* painter, const QRectF& rect) override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    bool viewportEvent(QEvent* event) override;

private:
    bool tileAt(QMouseEvent* event, int& x, int& y) const;

    const ZoneMap& zone;
    const Sprites& sprites;
    QGraphicsScene* scene;
    std::vector<QGraphicsPixmapItem*> tileItems;
    std::vector<QGraphicsPixmapItem*> elementItems;
    QGraphicsRectItem* ghostArea;
    QGraphicsPixmapItem* ghostSprite;
};

#endif  // ZONE_CANVAS_H