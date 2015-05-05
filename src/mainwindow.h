#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <qgeomappingmanager.h>

#include <QMainWindow>
#include <QGraphicsView>
#include <QGeoServiceProvider>
#include <qnetworksession.h>
#include <QPushButton>
#include <QGraphicsPixmapItem>
#include <QLabel>

#include "positionsource.h"

class QResizeEvent;
class QShowEvent;

QTM_USE_NAMESPACE
class MapWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void showEvent(QShowEvent *);

private:
    void setupMap();
    void setProvider(QString providerId);
    void showMenu();
    void closeMenu();
    void saveCurrentPosition();
    void drawSavedPositions();
    void drawSavedPosIndicator(QGeoCoordinate coord);
    void drawCurrLocIndicator();
    void clearIndicators();
    double mpsToMPH(double mps);

private slots:
    void networkSessionOpened();
    void error(QNetworkSession::SessionError error);
    void menuButtonClicked();
    void mapButtonClicked();
    void saveButtonClicked();
    void goToButtonClicked();
    void quitButtonClicked();
    void mapCenterChanged();
    void positionUpdated(const QGeoPositionInfo &info);
    void setMapPanMode();

private:
    QGeoServiceProvider *m_serviceProvider;
    QGeoMappingManager *m_mapManager;

    MapWidget*      m_mapWidget;

    QGraphicsView* m_qgv;

    QNetworkSession *m_session;

    QPushButton *m_menuButton;

    QPushButton *m_goToButton;

    QPushButton *m_savePosButton;
    QPushButton *m_mapButton;
    QPushButton *m_quitButton;

    QGraphicsPixmapItem *m_pinIndicator;

    QLabel *m_speedLabel;

    std::list<QGeoCoordinate> m_placesList;
    std::list<QGraphicsPixmapItem*> m_placeIndicatorList;

    PositionSource *m_positionSource;

    QPixmap m_pinPixmap;
    QPixmap m_currLocPinPixmap;

    QGraphicsPixmapItem *m_currLocPinItem;

    QGeoPositionInfo m_currPosition;
    bool m_currPositionValid;

    bool m_panMode;
};

#endif // MAINWINDOW_H
