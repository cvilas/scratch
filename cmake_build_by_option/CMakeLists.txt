cmake_minimum_required(VERSION 3.16)
include(cmake/helpers.cmake)

enumerate_robohive_modules(ROOT_PATH ${CMAKE_SOURCE_DIR})
message(STATUS "after enum _ROBOHIVE_MODULES_ENUMERATE_FLAG=${_ROBOHIVE_MODULES_ENUMERATE_FLAG}")
get_property(my_list_content GLOBAL PROPERTY ROBOHIVE_DECLARED_MODULES)
message(STATUS "Declared modules: ${my_list_content}")


return()

project(build_by_option)


get_property(my_list_content GLOBAL PROPERTY ENABLED_MODULES)
message(STATUS "Enabled modules: ${my_list_content}")

get_list_of_variables_starting_with("BUILD_" built_projects)
message(STATUS "Modules:")
foreach(proj IN LISTS built_projects)
    message(STATUS "  ${proj} : ${${proj}}")
endforeach()

get_list_of_variables_starting_with("MODULE_LOCATION_" mod_locations)
message(STATUS "locations:")
foreach(proj IN LISTS mod_locations)
    message(STATUS "  ${proj} : ${${proj}}")
endforeach()

get_list_of_variables_starting_with("MODULE_DEPENDS_" mod_deps)
message(STATUS "deps:")
foreach(proj IN LISTS mod_deps)
    message(STATUS "  ${proj} : ${${proj}}")
endforeach()

print_cmake_variables()