set(DEPS bea)
module(NAME see DEPENDS_ON_MODULES ${DEPS})

add_library(see see.cpp see.h)
target_include_directories(see PUBLIC "../module_bea")
target_link_libraries(see ${DEPS})