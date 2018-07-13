TEMPLATE = lib
QT += qml quick
CONFIG += plugin c++11

uri = dkqmlcomponents
TARGET = pluginlib
DESTDIR = $$OUT_PWD/$$uri

# Input
SOURCES += \
    plugin.cpp \
    custom_object_view.cpp \
    custom_object.cpp

HEADERS += \
    plugin.h \
    custom_object_view.h \
    custom_object.h
OTHER_FILES += ../readme.md

DISTFILES = qmldir \
    ../readme.md

# copy qmldir to modules directory
!equals(_PRO_FILE_PWD_, $$OUT_PWD) {
    copy_qmldir.target = $$OUT_PWD/$$uri/qmldir
    copy_qmldir.depends = $$_PRO_FILE_PWD_/qmldir
    copy_qmldir.commands = $(COPY_FILE) \"$$replace(copy_qmldir.depends, /, $$QMAKE_DIR_SEP)\" \"$$replace(copy_qmldir.target, /, $$QMAKE_DIR_SEP)\"
    QMAKE_EXTRA_TARGETS += copy_qmldir
    PRE_TARGETDEPS += $$copy_qmldir.target
}

qmldir.files = qmldir
unix {
    installPath = $$[QT_INSTALL_QML]/$$replace(uri, \\., /)
    qmldir.path = $$installPath
    target.path = $$installPath
    INSTALLS += target qmldir
}
