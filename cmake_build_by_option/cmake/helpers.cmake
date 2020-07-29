# declare global list of declared modules
define_property(GLOBAL PROPERTY DECLARED_MODULES INHERITED
        BRIEF_DOCS "All the modules in the system"
        FULL_DOCS "All the modules in the system")
set_property(GLOBAL PROPERTY DECLARED_MODULES "")

# declare global list of modules marked for build
define_property(GLOBAL PROPERTY ENABLED_MODULES INHERITED
        BRIEF_DOCS "Modules marked to be built"
        FULL_DOCS "Modules marked to be built")
set_property(GLOBAL PROPERTY ENABLED_MODULES "")

# macro to declare a module
macro(module)
    set(flags "")
    set(single_opts NAME)
    set(multi_opts DEPENDS_ON_MODULES)

    include(CMakeParseArguments)
    cmake_parse_arguments(PACKAGE_ARG
            "${flags}"
            "${single_opts}"
            "${multi_opts}"
            ${ARGN})

    if(PACKAGE_ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unparsed arguments: ${PACKAGE_ARG_UNPARSED_ARGUMENTS}")
    endif()

    if(NOT PACKAGE_ARG_NAME)
        message(FATAL_ERROR "Project name not specified")
    endif()

    set_property(GLOBAL APPEND PROPERTY DECLARED_MODULES ${PACKAGE_ARG_NAME})
    set(MODULE_LOCATION_${PACKAGE_ARG_NAME} "${CMAKE_CURRENT_LIST_DIR}" CACHE INTERNAL "location of ${PACKAGE_ARG_NAME}")
    set(MODULE_DEPENDS_${PACKAGE_ARG_NAME} ${PACKAGE_ARG_DEPENDS_ON_MODULES} CACHE INTERNAL "Dependencies of ${PACKAGE_ARG_NAME}")

    message(STATUS "${PACKAGE_ARG_NAME} first pass")
    if(FIRST_PASS)
        return()
    endif()
    message(STATUS "${PACKAGE_ARG_NAME} second pass")

    # Configuration-time user option to build this project
    option(BUILD_${PACKAGE_ARG_NAME} "Build module ${PACKAGE_ARG_NAME}" OFF)
    if(NOT BUILD_${PACKAGE_ARG_NAME})
        return()
    endif()

    project(${PACKAGE_ARG_NAME} CXX)

    mark_module_to_build(${PACKAGE_ARG_NAME})
    foreach(dep IN LISTS PACKAGE_ARG_DEPENDS_ON_MODULES)
        mark_module_to_build(${dep})
    endforeach()

endmacro()

# macro to mark a module for build
macro(mark_module_to_build _project_name)
    message(STATUS "mark_project_to_build(${_project_name}) called by ${PROJECT_NAME}")
    set(BUILD_${_project_name} ON CACHE BOOL "" FORCE)
    set_property(GLOBAL APPEND PROPERTY ENABLED_MODULES ${_project_name})
    remove_duplicates_in_global_list(ENABLED_MODULES)

    # add project to global list
endmacro()

function(remove_duplicates_in_global_list _list)
    get_property(_list_content GLOBAL PROPERTY ${_list})
    list(REMOVE_DUPLICATES _list_content)
    set_property(GLOBAL PROPERTY ${_list} ${_list_content})
endfunction()

# Get a list of variables with specified prefix
function (get_list_of_variables_starting_with _prefix _varResult)
    get_cmake_property(_vars VARIABLES)
    string (REGEX MATCHALL "(^|;)${_prefix}[A-Za-z0-9_]*" _matchedVars "${_vars}")
    set (${_varResult} ${_matchedVars} PARENT_SCOPE)
endfunction()

macro(print_cmake_variables)
    get_cmake_property(_variableNames VARIABLES)
    list (SORT _variableNames)
    foreach (_variableName ${_variableNames})
        message(STATUS "${_variableName}=${${_variableName}}")
    endforeach()
endmacro()

file_contains_string(STRING "robohive_module"
        INPUT_LIST list_input
        OUTPUT list_out)

function(list_subdirectories _result _current_dir)
    FILE(GLOB children RELATIVE ${_current_dir} ${_current_dir}/*)
    SET(dirlist "")
    FOREACH(child ${children})
        IF(IS_DIRECTORY ${_current_dir}/${child})
            LIST(APPEND dirlist ${child})
        ENDIF()
    ENDFOREACH()
    SET(${_result} ${dirlist})
endfunction()

function(files_containing_string)
    set(flags "")
    set(single_opts STRING OUTPUT_LIST)
    set(multi_opts INPUT_LIST)

    include(CMakeParseArguments)
    cmake_parse_arguments(FUNC_ARG
            "${flags}"
            "${single_opts}"
            "${multi_opts}"
            ${ARGN})

    if(FUNC_ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unparsed arguments: ${FUNC_ARG_UNPARSED_ARGUMENTS}")
    endif()

    if(NOT FUNC_ARG_STRING)
        message(FATAL_ERROR "STRING not specified")
    endif()

    SET(files_list)
    foreach(_file ${FUNC_ARG_INPUT_LIST})
        file(STRINGS "${_file}" lines REGEX "${FUNC_ARG_STRING}")
        if(lines)
            list(APPEND files_list "${_file}")
        endif()
    endforeach()
    set(${OUTPUT} ${files_list})
endfunction()
