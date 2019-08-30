#
#  QCustomPlot Plot Examples
#

QT       += core gui opengl
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

DEFINES += QCUSTOMPLOT_USE_OPENGL RELEASE

TARGET = plot-examples
TEMPLATE = app

SOURCES += main.cpp\
           window.cpp \
           qcustomplot.cpp

HEADERS  += window.h \
         qcustomplot.h

FORMS    += mainwindow.ui

