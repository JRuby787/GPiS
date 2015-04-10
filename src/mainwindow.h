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
    void resizeEvent(QResizeEvent* event);
    void showEvent(QShowEvent *);

private:
    void setupMap();
    void setProvider(QString providerId);

private slots:
    void networkSessionOpened();
    void error(QNetworkSession::SessionError error);
    void menuButtonClicked();

private:
    QGeoServiceProvider *m_serviceProvider;
    QGeoMappingManager *m_mapManager;

    MapWidget*      m_mapWidget;

    QGraphicsView* m_qgv;

    QNetworkSession *m_session;

    QPushButton *m_menuButton;

    QGraphicsPixmapItem *m_pinIndicator;
};

#endif // MAINWINDOW_H
