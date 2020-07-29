cmake_minimum_required(VERSION 3.16)
include(cmake/helpers.cmake)

project(build_by_option)
set(FIRST_PASS ON)
include(module_ai/CMakeLists.txt)
include(module_bea/CMakeLists.txt)
include(module_see/CMakeLists.txt)
include(module_see/module_see_one/CMakeLists.txt)
set(FIRST_PASS OFF)

add_subdirectory(module_ai)

get_property(my_list_content GLOBAL PROPERTY DECLARED_MODULES)
message(STATUS "Declared modules: ${my_list_content}")

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

#print_cmake_variables()