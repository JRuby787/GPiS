#include <QtCore>

#include <math.h>

#include "positionsource.h"

PositionSource::PositionSource(QObject *parent)
    : QGeoPositionInfoSource(parent),
      timer(new QTimer(this)),
      lastPositionValid(false),
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
        if (lastPositionValid)
        {
            //info.setAttribute(QGeoPositionInfo::GroundSpeed, getGroundSpeed(info));
            info.setAttribute(QGeoPositionInfo::GroundSpeed, newData->fix.speed);
            //cout << "DIRECTION = " << newData->fix.track << endl;
        }
        if (info.isValid()) {
            lastPosition = info;
            lastPositionValid = true;
            emit positionUpdated(info);
        }
    }
}

double PositionSource::degToRad(double deg)
{
    return deg * M_PI / 180;
}

double PositionSource::haversin(double angle)
{
    return sin(angle/2) * sin(angle/2);
}

double PositionSource::getGroundSpeed(QGeoPositionInfo &info)
{
    // based on the haversine formula: http://en.wikipedia.org/wiki/Haversine_formula 
    double lat2_rad = degToRad(info.coordinate().latitude());
    double long2_rad = degToRad(info.coordinate().longitude());
    double lat1_rad = degToRad(lastPosition.coordinate().latitude());
    double long1_rad = degToRad(lastPosition.coordinate().longitude());

    double h = haversin(lat2_rad - lat1_rad) + cos(lat1_rad) * cos(lat2_rad) * haversin(long2_rad - long1_rad);

    // radius of Earth = 6371000 meters
    double d = 2 * 6371000 * asin(sqrt(h));

    unsigned int time2 = info.timestamp().toTime_t(); // seconds since 1970-01-01T00:00:00 UTC
    unsigned int time1 = lastPosition.timestamp().toTime_t(); // seconds since 1970-01-01T00:00:00 UTC

    return d/(time2-time1); // m/s
}

