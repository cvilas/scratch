TEMPLATE = app
QT += qml quick

HEADERS += \
    wrapper.h

SOURCES += \
    main.cpp \
    wrapper.cpp

RESOURCES += \
    resources.qrc

INCLUDEPATH += ../pluginlib
LIBS += -L ../pluginlib/dkqmlcomponents -lpluginlib

#DISTFILES += \
#    qml/qmloscilloscope/*

#target.path = $$[QT_INSTALL_EXAMPLES]/charts/qmloscilloscope
#INSTALLS += target

DISTFILES +=
