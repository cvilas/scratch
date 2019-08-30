# Qt project file - qmake uses his to generate a Makefile

QT       += core gui opengl

CONFIG          += qt warn_on release

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QwtExample

greaterThan(QT_MAJOR_VERSION, 4): LIBS += -lqwt-qt5 -lm
lessThan(QT_MAJOR_VERSION, 5): LIBS += -lqwt -lm

HEADERS += window.h

SOURCES += main.cpp window.cpp
