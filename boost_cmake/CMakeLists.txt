# Demonstrates how to pull down and build boost packages for a target architecture

cmake_minimum_required(VERSION 3.6)

include(ExternalProject)

project(boost_ext_example)

# turn on c++14 compile flags. Don't use non-standard extensions to iso cpp
# NOTE: external projects may not apply the same flags
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS OFF)

#------------------------------------------------------------------------------
# project wide settings for directories
#------------------------------------------------------------------------------
set(PLATFORM ${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR})
message(STATUS "PLATFORM: ${PLATFORM}")
set(PLATFORM_LOCAL_PATH ${CMAKE_SOURCE_DIR}/platforms/${PLATFORM})
if( CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT )
  file(MAKE_DIRECTORY ${PLATFORM_LOCAL_PATH}) # create directory to put platform specific files
  set(CMAKE_INSTALL_PREFIX ${PLATFORM_LOCAL_PATH} CACHE STRING "default install path" FORCE)
else()
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PLATFORM_LOCAL_PATH}/lib)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PLATFORM_LOCAL_PATH}/lib)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PLATFORM_LOCAL_PATH}/bin)
endif()
message(STATUS "Install path: ${CMAKE_INSTALL_PREFIX}")

#------------------------------------------------------------------------------
# project wide configuration parameters
#------------------------------------------------------------------------------
if( NOT CMAKE_BUILD_TYPE )
  set( CMAKE_BUILD_TYPE Debug CACHE STRING
       "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
       FORCE )
endif()
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

set(THIRDPARTY_CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})

IF(${CMAKE_SYSTEM_NAME} MATCHES "Android")
    set(BOOST_TOOLSET gcc-arm)
    set(BOOST_ARCH arm)
    set(THIRDPARTY_CMAKE_ARGS ${THIRDPARTY_CMAKE_ARGS} -DBUILD_SHARED_LIBS=OFF -DBUILD_STATIC_LIBS=ON)
    set(BUILD_SHARED_LIBS OFF)
    set(BUILD_STATIC_LIBS ON)
ELSE() # Linux
    set(BOOST_TOOLSET gcc)
    set(BOOST_ARCH " ")
    set(THIRDPARTY_CMAKE_ARGS ${THIRDPARTY_CMAKE_ARGS} -DBUILD_SHARED_LIBS=ON -DBUILD_STATIC_LIBS=OFF)
    set(BUILD_SHARED_LIBS ON)
    set(BUILD_STATIC_LIBS OFF)
ENDIF()

#------------------------------------------------------------------------------
# Boost
#------------------------------------------------------------------------------
SET (Boost_INCLUDE_DIRS ${CMAKE_INSTALL_PREFIX}/include/boost/)
SET (Boost_LIBRARIES ${CMAKE_INSTALL_PREFIX}/lib )
SET (CHRONO_STATIC_LIBRARIES ${CMAKE_INSTALL_PREFIX}/lib/libboost_chrono.a)
SET (DATETIME_STATIC_LIBRARIES ${CMAKE_INSTALL_PREFIX}/lib/libboost_date_time.a)
SET (SYSTEM_STATIC_LIBRARIES ${CMAKE_INSTALL_PREFIX}/lib/libboost_system.a)
SET (REGEX_STATIC_LIBRARIES ${CMAKE_INSTALL_PREFIX}/lib/libboost_regex.a)
SET (THREAD_STATIC_LIBRARIES ${CMAKE_INSTALL_PREFIX}/lib/libboost_thread.a)

ExternalProject_Add( Boost
    PREFIX Boost
    GIT_REPOSITORY https://github.com/boostorg/boost.git
    GIT_TAG boost-1.64.0
    GIT_SUBMODULES  libs/atomic libs/ratio libs/tuple libs/math libs/exception libs/intrusive libs/optional libs/chrono libs/asio libs/date_time libs/config libs/core  libs/detail  libs/io libs/iterator libs/predef libs/preprocessor  libs/smart_ptr libs/throw_exception libs/system libs/integer tools/build tools/bcp libs/serialization libs/interprocess libs/tokenizer libs/algorithm libs/program_options  libs/regex  libs/static_assert  libs/thread  libs/utility libs/numeric libs/range libs/array libs/bind libs/concept_check libs/lexical_cast libs/functional libs/assert libs/type_traits libs/mpl libs/move libs/container libs/any libs/type_index libs/function

    UPDATE_COMMAND ./bootstrap.sh --with-libraries=chrono,date_time,thread,system,regex --includedir=${PLATFORM_LOCAL_PATH}/include/
    CONFIGURE_COMMAND ./b2 toolset=${BOOST_TOOLSET} headers
    BUILD_COMMAND ./b2 toolset=${BOOST_TOOLSET} cxxflags=-fPIC cflags=-fPIC link=static install --libdir=${PLATFORM_LOCAL_PATH}/lib
    BUILD_IN_SOURCE 1
    INSTALL_COMMAND ""
)

# create temporary user configuration file before configuring boost
ExternalProject_Add_Step(
    Boost
    add_user_config
    COMMENT "Creating user configuration file user-config.jam"
    COMMAND ${CMAKE_COMMAND} -E make_directory <SOURCE_DIR>/tools/build/src/
    COMMAND echo "using gcc : ${BOOST_ARCH} : ${CMAKE_CXX_COMPILER} $<SEMICOLON>" > <SOURCE_DIR>/tools/build/src/user-config.jam
    DEPENDEES update download patch
    DEPENDERS configure
)

# Remove temporary user configuration file after installation
ExternalProject_Add_Step(
    Boost
    remove_user_config
    COMMENT "Removing user configuration file user-config.jam"
    COMMAND bash "-c" "rm <SOURCE_DIR>/tools/build/src/user-config.jam"
    DEPENDEES update download patch configure build install
)

ADD_LIBRARY (Boost_LIB STATIC IMPORTED DEPENDS Boost)
SET_TARGET_PROPERTIES (Boost_LIB PROPERTIES IMPORTED_LOCATION ${CHRONO_STATIC_LIBRARIES})
SET_TARGET_PROPERTIES (Boost_LIB PROPERTIES IMPORTED_LOCATION ${DATETIME_STATIC_LIBRARIES})
SET_TARGET_PROPERTIES (Boost_LIB PROPERTIES IMPORTED_LOCATION ${SYSTEM_STATIC_LIBRARIES})
SET_TARGET_PROPERTIES (Boost_LIB PROPERTIES IMPORTED_LOCATION ${REGEX_STATIC_LIBRARIES})
SET_TARGET_PROPERTIES (Boost_LIB PROPERTIES IMPORTED_LOCATION ${THREAD_STATIC_LIBRARIES})

