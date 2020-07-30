set(DEPS ai)
robohive_module(NAME bea DEPENDS_ON_MODULES ${DEPS})

add_library(bea bea.cpp bea.h)
target_include_directories(bea PUBLIC "../module_ai")
target_link_libraries(bea ${DEPS})