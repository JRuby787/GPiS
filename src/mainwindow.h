#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <qgeomappingmanager.h>

#include <QMainWindow>
#include <QGraphicsView>
#include <QGeoServiceProvider>
#include <qnetworksession.h>
#include <QPushButton>
#include <QGraphicsPixmapItem>

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

private slots:
    void networkSessionOpened();
    void error(QNetworkSession::SessionError error);
    void menuButtonClicked();
    void mapButtonClicked();
    void saveButtonClicked();

private:
    QGeoServiceProvider *m_serviceProvider;
    QGeoMappingManager *m_mapManager;

    MapWidget*      m_mapWidget;

    QGraphicsView* m_qgv;

    QNetworkSession *m_session;

    QPushButton *m_menuButton;

    QPushButton *m_savePosButton;
    QPushButton *m_mapButton;

    QGraphicsPixmapItem *m_pinIndicator;

    std::list<QGeoCoordinate> m_placesList;
};

#endif // MAINWINDOW_H
