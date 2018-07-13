# set version
set(TYPEINFO_VERSION_MAJOR 0)
set(TYPEINFO_VERSION_MINOR 1)

# configure a header file to pass some of the CMake settings to source code
configure_file (
    "${PROJECT_SOURCE_DIR}/src/cmake_config.h.in"
    "${PROJECT_SOURCE_DIR}/src/cmake_config.h"
    )

# source files
set(HDR_LIST
    cmake_config.h
    TypeInfo.h
    TypeInfoT.h)

set(SRC_LIST
    TypeInfo.cpp)

# target
add_library(${PROJECT_NAME} STATIC ${SRC_LIST} ${HDR_LIST})
