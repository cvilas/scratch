include(../common.pri)

TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

DEFINES += $${NDDS_DEFINES}
QMAKE_CXX = $${COMMON_CXX}
QMAKE_LINK = $${COMMON_LINKER}
QMAKE_CXXFLAGS += $${COMMON_CXXFLAGS}
QMAKE_LFLAGS += $${COMMON_LFLAGS}
DESTDIR = $${PWD}/../out/

SOURCES += main.cpp \
    EmbeddedSystem.cpp

HEADERS += \
    EmbeddedSystem.h

LIBS += -L$${PWD}/../out/ -lMessenger \
        $${NDDSLIBS}

INCLUDEPATH += $${NDDSINCLUDE} $$PWD/../messenger
DEPENDPATH += $$PWD/../messenger $$PWD/../types
