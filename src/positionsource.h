#ifndef POSITIONSOURCE_H
#define POSITIONSOURCE_H

#include <qgeopositioninfosource.h>
#include <gps.h>
#include <libgpsmm.h>

QTM_USE_NAMESPACE

class QTimer;

class PositionSource : public QGeoPositionInfoSource
{
    Q_OBJECT
public:
    PositionSource(QObject *parent = 0);

    QGeoPositionInfo lastKnownPosition(bool fromSatellitePositioningMethodsOnly = false) const;

    PositioningMethods supportedPositioningMethods() const;
    int minimumUpdateInterval() const;

public slots:
    virtual void startUpdates();
    virtual void stopUpdates();

    virtual void requestUpdate(int timeout = 5000);

private slots:
    void readNextPosition();

private:
    QTimer *timer;
    QGeoPositionInfo lastPosition;

    gpsmm gps_rec;
    struct gps_data_t* newData;
};

#endif
