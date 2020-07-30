#=======================================================================================================================
# Recurse through directory tree and find all CMakeLists.txt files that contains robohive module declaration
function(find_robohive_module_declarations _result)
    set(flags "")
    set(single_opts ROOT_PATH)
    set(multi_opts "")

    # This is the regex string we search for in the files to find a robohive module declaration
    set(_declaration_search_regex "robohive_module\\((NAME)")

    include(CMakeParseArguments)
    cmake_parse_arguments(_ARG
            "${flags}"
            "${single_opts}"
            "${multi_opts}"
            ${ARGN})

    if(_ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unparsed arguments: ${_ARG_UNPARSED_ARGUMENTS}")
    endif()

    if(NOT _ARG_ROOT_PATH)
        message(FATAL_ERROR "ROOT_PATH not specified.")
    endif()

    # - Find all cmakelists.txt recursively starting from specified path
    # - From these, extract those that contain 'robohive_module' string
    file(GLOB_RECURSE _all_cmakelists "${_ARG_ROOT_PATH}/CMakeLists.txt")

    SET(_module_files_list)
    foreach(_file ${_all_cmakelists})
        file(STRINGS "${_file}" _lines REGEX ${_declaration_search_regex})
        if(_lines)
            list(APPEND _module_files_list "${_file}")
        endif()
    endforeach()
    set(${_result} ${_module_files_list} PARENT_SCOPE)

endfunction()

#=======================================================================================================================
# Recurse through source tree and gather information about declared robohive modules
function(enumerate_robohive_modules)
    set(flags "")
    set(single_opts ROOT_PATH)
    set(multi_opts "")
    include(CMakeParseArguments)

    cmake_parse_arguments(_ARG
            "${flags}"
            "${single_opts}"
            "${multi_opts}"
            ${ARGN})

    if(_ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unparsed arguments: ${_ARG_UNPARSED_ARGUMENTS}")
    endif()

    if(NOT _ARG_ROOT_PATH)
        message(FATAL_ERROR "ROOT_PATH not specified.")
    endif()

    message(STATUS "enumerate called from ${CMAKE_CURRENT_LIST_FILE}")

    # Run through all module files collecting information about them
    set(_module_paths)
    find_robohive_module_declarations(_module_paths ROOT_PATH ${_ARG_ROOT_PATH})
    set(_ROBOHIVE_MODULES_ENUMERATE_FLAG ON CACHE INTERNAL "Enumeration of modules in progress")
    message(STATUS "Enumerating module declarations:")
    foreach(_module IN LISTS _module_paths)
        if(NOT ${_module} STREQUAL ${CMAKE_CURRENT_LIST_FILE}) # avoid recursion
            message(STATUS "  ${_module}")
            include(${_module})
        endif()
    endforeach()
    set(_ROBOHIVE_MODULES_ENUMERATE_FLAG OFF CACHE INTERNAL "")

    get_property(_declared_modules_list GLOBAL PROPERTY ROBOHIVE_DECLARED_MODULES)
    message(STATUS "Modules detected: ${_declared_modules_list}")

    # Go through list of enabled modules and mark it and its dependencies to be built
    mark_modules_for_build()

    # Add each marked module into the build
    get_property(_enabled_modules_list GLOBAL PROPERTY ROBOHIVE_ENABLED_MODULES)
    foreach(_m IN LISTS _enabled_modules_list)
        add_subdirectory(${ROBOHIVE_MODULE_${_m}_PATH})
    endforeach()
    message(STATUS "Modules enabled (including dependencies): ${_enabled_modules_list}")

endfunction()

#=======================================================================================================================
# declare global list of declared modules
define_property(GLOBAL PROPERTY ROBOHIVE_DECLARED_MODULES INHERITED
        BRIEF_DOCS "All the declared robohive modules in the system"
        FULL_DOCS "All the declared robohive modules in the system")
set_property(GLOBAL PROPERTY ROBOHIVE_DECLARED_MODULES "")

#=======================================================================================================================
# macro to declare a module
macro(robohive_module)
    set(flags "")
    set(single_opts NAME)
    set(multi_opts DEPENDS_ON_MODULES)

    include(CMakeParseArguments)
    cmake_parse_arguments(MODULE_ARG
            "${flags}"
            "${single_opts}"
            "${multi_opts}"
            ${ARGN})

    if(MODULE_ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unparsed arguments: ${MODULE_ARG_UNPARSED_ARGUMENTS}")
    endif()

    if(NOT MODULE_ARG_NAME)
        message(FATAL_ERROR "NAME not specified")
    endif()

    # Configuration-time user option to build this project
    option(BUILD_ROBOHIVE_MODULE_${MODULE_ARG_NAME} "Build module ${MODULE_ARG_NAME}" OFF)

    # if we are just enumerating modules, then set module meta information and exit
    # create global variables:
    #   ROBOHIVE_DECLARED_MODULES
    #   BUILD_<robohive_module>
    #   ROBOHIVE_MODULE_<robohive_module>_PATH
    #   ROBOHIVE_MODULE_<robohive_module>_DEPENDS_ON
    if(_ROBOHIVE_MODULES_ENUMERATE_FLAG)
        set_property(GLOBAL APPEND PROPERTY ROBOHIVE_DECLARED_MODULES ${MODULE_ARG_NAME})
        set(ROBOHIVE_MODULE_${MODULE_ARG_NAME}_PATH "${CMAKE_CURRENT_LIST_DIR}" CACHE INTERNAL "location of ${MODULE_ARG_NAME}")
        set(ROBOHIVE_MODULE_${MODULE_ARG_NAME}_DEPENDS_ON ${MODULE_ARG_DEPENDS_ON_MODULES} CACHE INTERNAL "Dependencies of ${MODULE_ARG_NAME}")

        # NOTE: Don't process the rest of the file in which this macro was called
        return()
    endif()
endmacro()

#=======================================================================================================================
# declare global list of modules marked for build
define_property(GLOBAL PROPERTY ROBOHIVE_ENABLED_MODULES INHERITED
        BRIEF_DOCS "Robohive modules marked for build either directly or to satisfy dependencies"
        FULL_DOCS "Robohive modules marked for build either directly or to satisfy dependencies")
set_property(GLOBAL PROPERTY ROBOHIVE_ENABLED_MODULES "")

#=======================================================================================================================
# Recursively walk through all enabled modules and their dependencies and mark them for building
function(mark_modules_for_build)

    # walk through modules list and tag those explicity marked for build
    get_property(_declared_modules GLOBAL PROPERTY ROBOHIVE_DECLARED_MODULES)
    foreach(_m IN LISTS _declared_modules)
        if(${BUILD_ROBOHIVE_MODULE_${_m}})
            mark_single_module_for_build(NAME ${_m})
            foreach(_dep IN LISTS ROBOHIVE_MODULE_${_m}_DEPENDS_ON)
                mark_single_module_for_build(NAME ${_dep})
            endforeach()
        endif()
    endforeach()

    # revisit the marked modules and resolve dependencies recursively
    set(_num_items_prev 0)
    set(_num_items 1)
    while(NOT(_num_items_prev EQUAL _num_items))
        set(_num_items_prev ${_num_items})
        get_property(_enabled_modules GLOBAL PROPERTY ROBOHIVE_ENABLED_MODULES)
        list(LENGTH _enabled_modules _num_items)
        foreach(_m IN LISTS _enabled_modules)
            foreach(_dep IN LISTS ROBOHIVE_MODULE_${_m}_DEPENDS_ON)
                mark_single_module_for_build(NAME ${_dep})
            endforeach()
        endforeach()
    endwhile()
endfunction()

#=======================================================================================================================
# Mark a single module for build
function(mark_single_module_for_build)
    set(flags "")
    set(single_opts NAME)
    set(multi_opts "")

    include(CMakeParseArguments)
    cmake_parse_arguments(_ARG
            "${flags}"
            "${single_opts}"
            "${multi_opts}"
            ${ARGN})

    if(_ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unparsed arguments: ${MODULE_ARG_UNPARSED_ARGUMENTS}")
    endif()

    if(NOT _ARG_NAME)
        message(FATAL_ERROR "NAME not specified")
    endif()

    set_property(GLOBAL APPEND PROPERTY ROBOHIVE_ENABLED_MODULES ${_ARG_NAME})
    remove_duplicates_in_global_list(ROBOHIVE_ENABLED_MODULES)
endfunction()

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

