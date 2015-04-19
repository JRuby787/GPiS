#ifndef MAPWIDGET_H
#define MAPWIDGET_H

#include <qgraphicsgeomap.h>

QTM_USE_NAMESPACE

class MapWidget : public QGraphicsGeoMap
{
    Q_OBJECT
public:
    MapWidget(QGeoMappingManager *manager);
    ~MapWidget();
signals:
    void mapPanMode();
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event);

private:
    bool m_panActive;
};

#endif // MAPWIDGET_H

