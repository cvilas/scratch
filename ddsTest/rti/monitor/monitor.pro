include(../common.pri)

QT       += core gui charts
greaterThan(QT_MAJOR_VERSION, 5): QT += widgets
CONFIG += console qt c++11

TARGET = monitor
TEMPLATE = app

DEFINES += $${NDDS_DEFINES}
QMAKE_CXX = $${COMMON_CXX}
QMAKE_LINK = $${COMMON_LINKER}
QMAKE_CXXFLAGS += $${COMMON_CXXFLAGS}
QMAKE_LFLAGS += $${COMMON_LFLAGS}
DESTDIR = $${PWD}/../out/

SOURCES += main.cpp \
    MainWindow.cpp \
    Plot.cpp

HEADERS += \
    MainWindow.h \
    Plot.h \
    DoubleBuffer.h

LIBS += -L$${PWD}/../out/ -lMessenger \
        $${NDDSLIBS}

INCLUDEPATH += $${NDDSINCLUDE} $$PWD/../messenger
DEPENDPATH += $$PWD/../messenger

FORMS += \
    MainWindow.ui

