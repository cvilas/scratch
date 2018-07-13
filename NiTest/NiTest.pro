TEMPLATE = app
CONFIG += gui console

INCLUDEPATH += /usr/include/ni $(OPENNI)/Include
SOURCES += main.cpp


unix:LIBS += -L/usr/lib/ -lOpenNI
win32:LIBS += -L$(OPENNI)/Lib -lOpenNI

