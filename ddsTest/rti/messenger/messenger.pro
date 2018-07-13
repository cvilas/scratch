include(../common.pri)

TEMPLATE = lib
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

TARGET = Messenger

SOURCES += Messenger.cpp \
    MonitorMessenger.cpp \
    PlantMessenger.cpp \
    ControlUpdater.cpp
HEADERS += Messenger.h \
    LogVariable.h \
    ControlVariable.h \
    PlantMessenger.h \
    MonitorMessenger.h \
    ControlVariable.hpp \
    LogVariable.hpp \
    ControlMonitor.h \
    LogMonitor.h \
    LogPublisher.h \
    ControlUpdater.h
INCLUDEPATH += $${NDDSINCLUDE} $$PWD/..
LIBS += $${NDDSLIBS}

DEFINES += $${NDDS_DEFINES}
QMAKE_CXX = $${COMMON_CXX}
QMAKE_LINK = $${COMMON_LINKER}
QMAKE_CXXFLAGS += $${COMMON_CXXFLAGS}
QMAKE_LFLAGS += $${COMMON_LFLAGS}

# target directories
DESTDIR = $${PWD}/../out/

DISTFILES += \
    qos.xml

copydata.commands = cp $$PWD/qos.xml $$DESTDIR
first.depends = $(first) copydata
export(first.depends)
export(copydata.commands)
QMAKE_EXTRA_TARGETS += first copydata
