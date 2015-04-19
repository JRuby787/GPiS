#include "mainwindow.h"
#include "mapwidget.h"

#include <QApplication>
#include <QMessageBox>
#include <QGeoCoordinate>
#include <QProcessEnvironment>
#include <QUrl>
#include <QNetworkProxy>
#include <QNetworkProxyFactory>
#include <QTimer>
#include <QAction>
#include <QMenuBar>
#include <qnetworkconfigmanager.h>
#include <QPixmap>
#include <QIcon>
#include <QSize>
#include <QGeoBoundingBox>

#include "positionsource.h"

#define WIN_W 480
#define WIN_H 320

QTM_USE_NAMESPACE

MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent),
        m_serviceProvider(0),
        m_mapWidget(0),
        m_currLocPinItem(0),
        m_currPositionValid(false),
        m_panMode(false)
{
    // set main window properties
    setWindowTitle(tr("GPiS")); 
    setFixedSize(WIN_W, WIN_H);
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);

    // create a scene to display graphics objects for the map view
    QGraphicsScene *sc = new QGraphicsScene;
    m_qgv = new QGraphicsView(sc, this);
    m_qgv->setVisible(true);
    m_qgv->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_qgv->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setCentralWidget(m_qgv);

    // create a menu button
    m_menuButton = new QPushButton("Menu", this);
    m_menuButton->move((width()-m_menuButton->width())/2, height()-m_menuButton->height());
    m_menuButton->setVisible(true);
    connect(m_menuButton, SIGNAL(clicked()), this, SLOT(menuButtonClicked()));

    // create a "go to current location" button
    m_goToButton = new QPushButton(this);
    m_goToButton->resize(35, 35);
    QPixmap goToPixmap;
    goToPixmap.load(":/loc-icon.png");
    m_goToButton->setIcon(QIcon(goToPixmap));
    m_goToButton->setIconSize(QSize(35, 35));
    m_goToButton->move(width()-m_goToButton->width(), 0);
    m_goToButton->setVisible(true);
    connect(m_goToButton, SIGNAL(clicked()), this, SLOT(goToButtonClicked()));

    // create label for speed indicator
    m_speedLabel = new QLabel(this);
    m_speedLabel->resize(m_menuButton->size());
    m_speedLabel->move(width()-m_speedLabel->width(), height()-m_speedLabel->height());
    m_speedLabel->setAlignment(Qt::AlignCenter);
    m_speedLabel->setText("<b>0.0 MPH</b>");
    m_speedLabel->setAutoFillBackground(true);

    // load the pixmaps for the position indicators
    m_pinPixmap.load(":/map-pin-green.png");
    m_currLocPinPixmap.load(":/map-pin-blue.png");

    // create the buttons for the main menu
    // save current position button
    m_savePosButton = new QPushButton(this);
    m_savePosButton->setVisible(false); 
    m_savePosButton->resize(80, 80); 
    QPixmap savePixmap;
    savePixmap.load(":/save-icon.png");
    m_savePosButton->setIcon(QIcon(savePixmap));
    m_savePosButton->setIconSize(QSize(60, 60));
    m_savePosButton->move(width()/2, height()/6);
    connect(m_savePosButton, SIGNAL(clicked()), this, SLOT(saveButtonClicked()));
    // return to map view button
    m_mapButton = new QPushButton(this);
    m_mapButton->setVisible(false); 
    m_mapButton->resize(80, 80); 
    QPixmap mapPixmap;
    mapPixmap.load(":/map-icon.png");
    m_mapButton->setIcon(QIcon(mapPixmap));
    m_mapButton->setIconSize(QSize(60, 60));
    m_mapButton->move(width()/6, height()/6);
    connect(m_mapButton, SIGNAL(clicked()), this, SLOT(mapButtonClicked()));

    // set Internet Access Point
    QNetworkConfigurationManager manager;
    const bool canStartIAP = (manager.capabilities()
                              & QNetworkConfigurationManager::CanStartAndStopInterfaces);

    // if there is a default access point, use it
    QNetworkConfiguration cfg = manager.defaultConfiguration();
    if (!cfg.isValid() || (!canStartIAP && cfg.state() != QNetworkConfiguration::Active)) {
        QMessageBox::information(this, tr("GPiS"), tr(
                                     "Available Access Points not found."));
        return;
    }

    // set callbacks and start internet connection
    m_session = new QNetworkSession(cfg, this);
    connect(m_session, SIGNAL(opened()), this, SLOT(networkSessionOpened()));
    connect(m_session,SIGNAL(error(QNetworkSession::SessionError)),this,SLOT(error(QNetworkSession::SessionError)));
    m_session->open();

    // create new position source and start position updates
    m_positionSource = new PositionSource(this);
    connect(m_positionSource, SIGNAL(positionUpdated(QGeoPositionInfo)), this, SLOT(positionUpdated(QGeoPositionInfo)));
    m_positionSource->setUpdateInterval(5000);
    m_positionSource->startUpdates();
}

MainWindow::~MainWindow()
{
    delete m_serviceProvider;
}

void MainWindow::showEvent(QShowEvent* event)
{
    m_qgv->setSceneRect(QRectF(QPointF(0.0, 0.0), m_qgv->size()));
    if (m_mapWidget)
        m_mapWidget->resize(m_qgv->size());
}

void MainWindow::networkSessionOpened()
{
    QString urlEnv = QProcessEnvironment::systemEnvironment().value("http_proxy");
    if (!urlEnv.isEmpty()) {
        QUrl url = QUrl(urlEnv, QUrl::TolerantMode);
        QNetworkProxy proxy;
        proxy.setType(QNetworkProxy::HttpProxy);
        proxy.setHostName(url.host());
        proxy.setPort(url.port(8080));
        QNetworkProxy::setApplicationProxy(proxy);
    } else
        QNetworkProxyFactory::setUseSystemConfiguration(true);

    // get provider, we are hardcoding it to nokia
    setProvider("nokia");
    // set up the map widget
    setupMap();
}

void MainWindow::error(QNetworkSession::SessionError error)
{
    if (error == QNetworkSession::UnknownSessionError) {
        if (m_session->state() == QNetworkSession::Connecting) {
            QMessageBox msgBox(qobject_cast<QWidget *>(parent()));
            msgBox.setText("This application requires network access to function.");
            msgBox.setInformativeText("Press Cancel to quit the application.");
            msgBox.setIcon(QMessageBox::Information);
            msgBox.setStandardButtons(QMessageBox::Retry | QMessageBox::Cancel);
            msgBox.setDefaultButton(QMessageBox::Retry);
            int ret = msgBox.exec();
            if (ret == QMessageBox::Retry) {
                QTimer::singleShot(0, m_session, SLOT(open()));
            } else if (ret == QMessageBox::Cancel) {
                close();
            }
        }
    } else if (error == QNetworkSession::SessionAbortedError) {
        QMessageBox msgBox(qobject_cast<QWidget *>(parent()));
        msgBox.setText("Out of range of network.");
        msgBox.setInformativeText("Move back into range and press Retry, or press Cancel to quit the application.");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setStandardButtons(QMessageBox::Retry | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Retry);
        int ret = msgBox.exec();
        if (ret == QMessageBox::Retry) {
            QTimer::singleShot(0, m_session, SLOT(open()));
        } else if (ret == QMessageBox::Cancel) {
            close();
        }
    }
}

void MainWindow::setProvider(QString providerId)
{
    if (m_serviceProvider)
        delete m_serviceProvider;
    m_serviceProvider = new QGeoServiceProvider(providerId);
    if (m_serviceProvider->error() != QGeoServiceProvider::NoError) {
        QMessageBox::information(this, tr("GPiS"), tr(
                                     "Unable to find the %1 geoservices plugin.").arg(providerId));
        qApp->quit();
        return;
    }

    m_mapManager = m_serviceProvider->mappingManager();
}

void MainWindow::setupMap()
{
    m_mapWidget = new MapWidget(m_mapManager);
    m_qgv->scene()->addItem(m_mapWidget);
    m_mapWidget->setCenter(QGeoCoordinate(40.74445,-74.02580));
    m_mapWidget->setZoomLevel(15); // valid levels: 0 (min) to 18 (max)
    connect(m_mapWidget, SIGNAL(centerChanged(const QGeoCoordinate)), this, SLOT(mapCenterChanged()));
    connect(m_mapWidget, SIGNAL(mapPanMode()), this, SLOT(setMapPanMode()));

    drawSavedPositions();

    resizeEvent(0);
}

void MainWindow::menuButtonClicked()
{
    // show the menu
    showMenu();
}

void MainWindow::showMenu()
{
    // hide the map QGraphicsView
    m_qgv->setVisible(false);

    // hide the menu button
    m_menuButton->setVisible(false);

    // hide the speed indicator
    m_speedLabel->setVisible(false);

    // hide the go-to current location button
    m_goToButton->setVisible(false);

    // show the menu
    m_savePosButton->setVisible(true); 
    m_mapButton->setVisible(true); 
}

void MainWindow::mapButtonClicked()
{
   // return to the map
   closeMenu(); 
}

void MainWindow::closeMenu()
{
    // show the map QGraphicsView
    m_qgv->setVisible(true);
    drawSavedPositions();

    // show the menu button
    m_menuButton->setVisible(true);
    
    // show the speed indicator
    m_speedLabel->setVisible(true);

    // show the go-to current location button
    m_goToButton->setVisible(true);

    // hide the menu
    m_savePosButton->setVisible(false); 
    m_mapButton->setVisible(false); 
}

void MainWindow::saveButtonClicked()
{
    saveCurrentPosition();
}

void MainWindow::goToButtonClicked()
{
    m_panMode = false;
    if (m_currPositionValid)
    {
        m_mapWidget->setCenter(m_currPosition.coordinate());
    }
}

void MainWindow::saveCurrentPosition()
{
    m_placesList.push_back(m_mapWidget->center());
}

void MainWindow::drawSavedPositions()
{
    QGeoBoundingBox mapArea = m_mapWidget->viewport();

    for (std::list<QGeoCoordinate>::iterator placeIter = m_placesList.begin();
         placeIter != m_placesList.end();
         ++placeIter)
    {
        if (mapArea.contains(*placeIter))
        {
            drawSavedPosIndicator(*placeIter);
        }
    }
}

void MainWindow::drawSavedPosIndicator(QGeoCoordinate coord)
{
    // draw the a saved location indicator
    QGraphicsPixmapItem *pinIndicator = m_qgv->scene()->addPixmap(m_pinPixmap);
    QPointF point = m_mapWidget->coordinateToScreenPosition(coord);
    pinIndicator->setOffset(point.x()-10, point.y()-32);
    m_placeIndicatorList.push_back(pinIndicator);
}

void MainWindow::clearIndicators()
{
    QGraphicsPixmapItem *pItem;

    while (!m_placeIndicatorList.empty())
    {
        pItem = m_placeIndicatorList.front();
        m_placeIndicatorList.pop_front();
        m_qgv->scene()->removeItem(pItem);
        delete pItem;
    }
}

void MainWindow::mapCenterChanged()
{
    clearIndicators();
    drawSavedPositions();
    drawCurrLocIndicator();
}

void MainWindow::drawCurrLocIndicator()
{
    // re-draw the current location indicator
    if (m_currLocPinItem)
    {
        m_qgv->scene()->removeItem(m_currLocPinItem);
        delete m_currLocPinItem;
        m_currLocPinItem = 0;
    }
    QGeoBoundingBox mapArea = m_mapWidget->viewport();
    if (mapArea.contains(m_currPosition.coordinate()))
    {
        m_currLocPinItem = m_pinIndicator = m_qgv->scene()->addPixmap(m_currLocPinPixmap);
        QPointF point = m_mapWidget->coordinateToScreenPosition(m_currPosition.coordinate());
        m_pinIndicator->setOffset(point.x()-10, point.y()-32);
    }
}

void MainWindow::positionUpdated(const QGeoPositionInfo &info)
{
    m_currPosition = info;
    m_currPositionValid = true;

    if (!m_panMode)
    {
        m_mapWidget->setCenter(m_currPosition.coordinate());
    }

    drawCurrLocIndicator();
    
    // update the velocity
    if (info.hasAttribute(QGeoPositionInfo::GroundSpeed))
    {
        m_speedLabel->setText("<b>" + QString::number(mpsToMPH(info.attribute(QGeoPositionInfo::GroundSpeed)), 'f', 1) + " MPH</b>");
    }
}

double MainWindow::mpsToMPH(double mps)
{
    // (x meter/sec) * (60 sec/min) * (60 min/hr) * (1 mile/1609.34 meter) => 1 m/s = 2.23694 mi/hr
    return mps * 2.23694;
}

void MainWindow::setMapPanMode()
{
    m_panMode = true;
}
