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

#include <iostream>
using std::cout;
using std::endl;

#define WIN_W 480
#define WIN_H 320

QTM_USE_NAMESPACE

MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent),
        m_serviceProvider(0),
        m_mapWidget(0)
{
    // set main window properties
    setWindowTitle(tr("GPiS")); 
    setFixedSize(WIN_W, WIN_H);

    // create a scene to display graphics objects for the map view
    QGraphicsScene *sc = new QGraphicsScene;
    m_qgv = new QGraphicsView(sc, this);
    m_qgv->setVisible(true);
    m_qgv->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_qgv->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setCentralWidget(m_qgv);

    // create a menu button
    m_menuButton = new QPushButton("Menu", this);
    m_menuButton->setVisible(true);
    connect(m_menuButton, SIGNAL(clicked()), this, SLOT(menuButtonClicked()));

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

    // if there default access point, use it
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

    m_menuButton->move((width()-m_menuButton->width())/2, height()-m_menuButton->height());
    m_pinIndicator->setOffset(width()/2-10, height()/2-32);
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
    m_mapWidget->setCenter(QGeoCoordinate(40.744454,-74.025798));
    m_mapWidget->setZoomLevel(15); // valid levels: 0 (min) to 18 (max)

    // create a pixmap for the current location indicator
    QPixmap pinPixmap;
    pinPixmap.load(":/map-pin.png");
    m_pinIndicator = m_qgv->scene()->addPixmap(pinPixmap);

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

    // hide the menu
    m_savePosButton->setVisible(false); 
    m_mapButton->setVisible(false); 
}

void MainWindow::saveButtonClicked()
{
    saveCurrentPosition();
}

void MainWindow::saveCurrentPosition()
{
    m_placesList.push_back(m_mapWidget->center());
}

void MainWindow::drawSavedPositions()
{
    // TODO: figure out what to do if this is called multiple times
    // I think we should probably remove all of the markers and re-draw them
    // since this function should get called every time the map is redrawn (i.e. when location changes)

    cout << "Draw saved positions" << endl;

    QGeoBoundingBox mapArea = m_mapWidget->viewport();

    for(
       std::list<QGeoCoordinate>::iterator placeIter = m_placesList.begin();
       placeIter != m_placesList.end();
       ++placeIter)
    {
        cout << "Place in list: " << placeIter->toString(QGeoCoordinate::Degrees).toStdString() << endl;
        if (mapArea.contains(*placeIter))
            cout << "Place is in view!" << endl;
            // TODO: place a marker at this coordinate
        else
            cout << "Place is NOT in view!" << endl;
            // TODO: nothing
    }
}
