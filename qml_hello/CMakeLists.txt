cmake_minimum_required(VERSION 3.17)

project(qml_hello)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Core Qml Quick REQUIRED)

add_executable(qml_hello
    main.cpp
    main.qrc
    main.qml
)

target_link_libraries(qml_hello Qt5::Core Qt5::Qml Qt5::Quick)
