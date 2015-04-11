QT += xml network svg
TARGET = GPiS
TEMPLATE = app 

DESTDIR = bin
OBJECTS_DIR = obj
MOC_DIR = moc
RCC_DIR = qrc

SOURCES += src/main.cpp \
           src/mapwidget.cpp \
           src/mainwindow.cpp \
           src/positionsource.cpp
           
HEADERS += src/mapwidget.h \
           src/mainwindow.h \
           src/positionsource.h

RESOURCES = rsrc/resources.qrc

CONFIG += mobility
MOBILITY = location

