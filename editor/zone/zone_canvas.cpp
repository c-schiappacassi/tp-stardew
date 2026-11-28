#include "zone_canvas.h"

#include <QEvent>
#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QPainter>

#include <algorithm>

namespace {
const int TILE_PIXELS = 16;
const QColor GRID_COLOR(0, 0, 0, 60);
const QColor CAN_PLACE_COLOR(0, 200, 0, 90);
const QColor CANNOT_PLACE_COLOR(220, 0, 0, 130);
const qreal GHOST_OPACITY = 0.6;
const qreal GHOST_Z = 10000;
}

ZoneCanvas::ZoneCanvas(const ZoneMap& zone, const Sprites& sprites, QWidget* parent):
        QGraphicsView(parent), zone(zone), sprites(sprites),
        scene(new QGraphicsScene(this)), floorLayer(zone.getWidth() * TILE_PIXELS, zone.getHeight() * TILE_PIXELS) {
    floorLayer.fill(Qt::black);
    QPainter painter(&floorLayer);
    for (int y = 0; y < zone.getHeight(); ++y) {
        for (int x = 0; x < zone.getWidth(); ++x) {
            drawTile(painter, x, y);
        }
    }
    painter.end();

    mapItem = scene->addPixmap(QPixmap());
    scene->setSceneRect(0, 0, zone.getWidth() * TILE_PIXELS, zone.getHeight() * TILE_PIXELS);
    setScene(scene);
    setMouseTracking(true);

    ghostArea = scene->addRect(0, 0, 0, 0, Qt::NoPen);
    ghostArea->setZValue(GHOST_Z);
    ghostSprite = scene->addPixmap(QPixmap());
    ghostSprite->setOpacity(GHOST_OPACITY);
    ghostSprite->setZValue(GHOST_Z);
    clearGhost();

    redrawMap();
}

void ZoneCanvas::drawTile(QPainter& painter, int x, int y) const {
    painter.drawPixmap(x * TILE_PIXELS, y * TILE_PIXELS, sprites.floorTile(zone.getFloor(x, y), x, y));
}

void ZoneCanvas::refreshTile(int x, int y) {
    QPainter painter(&floorLayer);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    drawTile(painter, x, y);
    painter.end();
    redrawMap();
}

void ZoneCanvas::refreshElements() {
    redrawMap();
}

void ZoneCanvas::redrawMap() {
    std::vector<PlacedElement> ordered = zone.getElements();
    std::stable_sort(ordered.begin(), ordered.end(), [](const PlacedElement& a, const PlacedElement& b) {
        const ElementInfo& infoA = ElementCatalog::infoOf(a.element);
        const ElementInfo& infoB = ElementCatalog::infoOf(b.element);
        if (infoA.onWall != infoB.onWall) {
            return infoB.onWall;
        }
        return a.y + infoA.tilesHeight < b.y + infoB.tilesHeight;
    });

    QPixmap map = floorLayer;
    QPainter painter(&map);
    for (const PlacedElement& placed: ordered) {
        const ElementInfo& info = ElementCatalog::infoOf(placed.element);
        const QPixmap& sprite = sprites.element(placed.element);
        painter.drawPixmap(placed.x * TILE_PIXELS + (info.tilesWidth * TILE_PIXELS - sprite.width()) / 2,
                           placed.y * TILE_PIXELS + info.tilesHeight * TILE_PIXELS - sprite.height(), sprite);
    }
    painter.end();
    mapItem->setPixmap(map);
}

void ZoneCanvas::showGhost(ElementId element, int x, int y, bool canPlace) {
    const ElementInfo& info = ElementCatalog::infoOf(element);
    const QPixmap& sprite = sprites.element(element);
    const QRect area(x * TILE_PIXELS, y * TILE_PIXELS, info.tilesWidth * TILE_PIXELS, info.tilesHeight * TILE_PIXELS);

    ghostArea->setRect(area.intersected(sceneRect().toRect()));
    ghostArea->setBrush(canPlace ? CAN_PLACE_COLOR : CANNOT_PLACE_COLOR);
    ghostArea->show();

    ghostSprite->setPixmap(sprite);
    ghostSprite->setPos(area.x() + (area.width() - sprite.width()) / 2, area.y() + area.height() - sprite.height());
    ghostSprite->setVisible(sceneRect().toRect().contains(area));
}

void ZoneCanvas::clearGhost() {
    ghostArea->hide();
    ghostSprite->hide();
}

void ZoneCanvas::drawForeground(QPainter* painter, const QRectF&) {
    painter->setPen(QPen(GRID_COLOR, 0));
    const int width = zone.getWidth() * TILE_PIXELS;
    const int height = zone.getHeight() * TILE_PIXELS;
    for (int x = 0; x <= width; x += TILE_PIXELS) {
        painter->drawLine(x, 0, x, height);
    }
    for (int y = 0; y <= height; y += TILE_PIXELS) {
        painter->drawLine(0, y, width, y);
    }
}

void ZoneCanvas::resizeEvent(QResizeEvent* event) {
    QGraphicsView::resizeEvent(event);
    fitInView(sceneRect(), Qt::KeepAspectRatio);
}

void ZoneCanvas::mousePressEvent(QMouseEvent* event) {
    int x = 0;
    int y = 0;
    if (event->button() == Qt::RightButton) {
        emit rightClicked();
    } else if (event->button() == Qt::LeftButton && tileAt(event, x, y)) {
        emit tileClicked(x, y);
    }
}

void ZoneCanvas::mouseMoveEvent(QMouseEvent* event) {
    int x = 0;
    int y = 0;
    if (!tileAt(event, x, y)) {
        emit hoverLeft();
        return;
    }
    emit tileHovered(x, y);
    if (event->buttons() & Qt::LeftButton) {
        emit tileDragged(x, y);
    }
}

bool ZoneCanvas::viewportEvent(QEvent* event) {
    if (event->type() == QEvent::Leave) {
        emit hoverLeft();
    }
    return QGraphicsView::viewportEvent(event);
}

bool ZoneCanvas::tileAt(QMouseEvent* event, int& x, int& y) const {
    const QPointF position = mapToScene(event->position().toPoint());
    if (position.x() < 0 || position.y() < 0) {
        return false;
    }
    x = static_cast<int>(position.x()) / TILE_PIXELS;
    y = static_cast<int>(position.y()) / TILE_PIXELS;
    return zone.inBounds(x, y);
}