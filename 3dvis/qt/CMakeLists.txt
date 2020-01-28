set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(robohive COMPONENTS REQUIRED wemo)
find_package(Eigen3)
find_package(Threads)
find_package(foonathan_memory)
find_package(Qt5 COMPONENTS Core Gui Widgets 3DCore 3DExtras 3DRender 3DInput REQUIRED)

add_executable(wemo_qt model/body.qrc wemo.cpp)
target_link_libraries(wemo_qt Qt5::Core Qt5::Gui Qt5::Widgets Qt5::3DCore Qt5::3DExtras Qt5::3DRender Qt5::3DInput robohive::wemo atomic)
