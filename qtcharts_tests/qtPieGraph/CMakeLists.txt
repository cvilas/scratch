# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(quickwidgetgraphs LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS Quick)
find_package(Qt6 COMPONENTS Graphs)
find_package(Qt6 COMPONENTS QuickWidgets)
find_package(Qt6 COMPONENTS Widgets)

qt_add_executable(quickwidgetgraphs
    main.cpp
    piewidget.h piewidget.cpp
    piegraph.h piegraph.cpp
)

target_link_libraries(quickwidgetgraphs PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Quick
    Qt::Graphs
    Qt::QuickWidgets
    Qt::Widgets
)

qt6_add_qml_module(quickwidgetgraphs
    URI QuickWidgetExample
    NO_RESOURCE_TARGET_PATH
    QML_FILES
        qml/quickwidgetgraphs/main.qml
)