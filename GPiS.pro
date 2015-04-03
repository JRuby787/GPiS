QT += xml network svg
TARGET = GPiS
TEMPLATE = app 

SOURCES += main.cpp \
           mapwidget.cpp \
           mainwindow.cpp
           
HEADERS += mapwidget.h \
           mainwindow.h

CONFIG += mobility
MOBILITY = location

equals(QT_MAJOR_VERSION, 4):lessThan(QT_MINOR_VERSION, 7){
    MOBILITY += bearer
    INCLUDEPATH += ../../src/bearer
}

