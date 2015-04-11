#include "mapwidget.h"

#include <QGraphicsSceneMouseEvent>

QTM_USE_NAMESPACE

MapWidget::MapWidget(QGeoMappingManager *manager) : QGraphicsGeoMap(manager)
{
    m_panActive = false;
}

MapWidget::~MapWidget() {}


void MapWidget::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    setFocus();
    if (event->button() == Qt::LeftButton) {
        m_panActive = true;
    }

    event->accept();
}

void MapWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        if (m_panActive) {
            m_panActive = false;
        }
    }

    event->accept();
}


void MapWidget::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (m_panActive) {
        pan(event->lastPos().x() - event->pos().x(), event->lastPos().y() - event->pos().y());
    }

    event->accept();
}

