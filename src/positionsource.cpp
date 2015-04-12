#include <QtCore>

#include <math.h>

#include "positionsource.h"

PositionSource::PositionSource(QObject *parent)
    : QGeoPositionInfoSource(parent),
      logFile(new QFile(this)),
      timer(new QTimer(this)),
      lastPositionValid(false)
{
    connect(timer, SIGNAL(timeout()), this, SLOT(readNextPosition()));

    logFile->setFileName(QCoreApplication::applicationDirPath()
            + QDir::separator() + ".." + QDir::separator() + "locations.txt");
    if (!logFile->open(QIODevice::ReadOnly))
        qWarning() << "Error: cannot open source file" << logFile->fileName();
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
    // For simplicity, ignore timeout - assume that if data is not available
    // now, no data will be added to the file later
    if (logFile->canReadLine())
        readNextPosition();
    else
        emit updateTimeout();
}

void PositionSource::readNextPosition()
{
    QByteArray line = logFile->readLine().trimmed();
    if (!line.isEmpty()) {
        QList<QByteArray> data = line.split(' ');
        double latitude;
        double longitude;
        bool hasLatitude = false;
        bool hasLongitude = false;
        QDateTime timestamp = QDateTime::fromString(QString(data.value(0)), Qt::ISODate);
        latitude = data.value(1).toDouble(&hasLatitude);
        longitude = data.value(2).toDouble(&hasLongitude);

        if (hasLatitude && hasLongitude && timestamp.isValid()) {
            QGeoCoordinate coordinate(latitude, longitude);
            QGeoPositionInfo info(coordinate, timestamp);
        if (lastPositionValid)
            {
                info.setAttribute(QGeoPositionInfo::GroundSpeed, getGroundSpeed(info));
            }
            if (info.isValid()) {
                lastPosition = info;
        lastPositionValid = true;
                emit positionUpdated(info);
            }
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

