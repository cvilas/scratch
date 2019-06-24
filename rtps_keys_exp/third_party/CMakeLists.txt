message(STATUS "Updating fastrtps ..")
execute_process(COMMAND git submodule update --init -- ${CMAKE_CURRENT_SOURCE_DIR}/fastrtps
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set(THIRDPARTY ON CACHE BOOL "Set to build third party libs for fastrtps")
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/fastrtps)
set(THIRDPARTY OFF CACHE BOOL "Set to build third party libs for fastrtps")
