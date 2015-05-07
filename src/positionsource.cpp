#include <QtCore>

#include <math.h>

#include "positionsource.h"

PositionSource::PositionSource(QObject *parent)
    : QGeoPositionInfoSource(parent),
      timer(new QTimer(this)),
      gps_rec("localhost", DEFAULT_GPSD_PORT)
{
    connect(timer, SIGNAL(timeout()), this, SLOT(readNextPosition()));

    if (gps_rec.stream(WATCH_ENABLE|WATCH_JSON) == NULL) {
        qWarning()  << "No GPSD running.";
    }
}

QGeoPositionInfo PositionSource::lastKnownPosition(bool /*fromSatellitePositioningMethodsOnly*/) const
{
    return lastPosition;
}

PositionSource::PositioningMethods PositionSource::supportedPositioningMethods() const
{
    return AllPositioningMethods;
}

int PositionSource::minimumUpdateInterval() const
{
    return 500;
}

void PositionSource::startUpdates()
{
    int interval = updateInterval();
    if (interval < minimumUpdateInterval())
        interval = minimumUpdateInterval();

    timer->start(interval);
}

void PositionSource::stopUpdates()
{
    timer->stop();
}

void PositionSource::requestUpdate(int /*timeout*/)
{
    readNextPosition();
}

void PositionSource::readNextPosition()
{
    if ((newData = gps_rec.read()) == NULL) {
        qWarning()  << "GPS read error.";
    }
    else if (newData->online) {
        double latitude = newData->fix.latitude;
        double longitude = newData->fix.longitude;

        QDateTime timestamp = QDateTime::fromTime_t(newData->fix.time);
        QGeoCoordinate coordinate(latitude, longitude);
        QGeoPositionInfo info(coordinate, timestamp);
        info.setAttribute(QGeoPositionInfo::GroundSpeed, newData->fix.speed);
#ifdef ENABLE_ROTATION
        info.setAttribute(QGeoPositionInfo::Direction, newData->fix.track);
#endif // ENABLE_ROTATION
        if (info.isValid()) {
            lastPosition = info;
            emit positionUpdated(info);
        }
    }
}

