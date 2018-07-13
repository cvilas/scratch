
include_directories(
    ${PROJECT_SOURCE_DIR}/src/)

link_directories(
    ${PROJECT_SOURCE_DIR}/lib)

add_executable(TypeInfoTest TypeInfoTest.cpp)
target_link_libraries(TypeInfoTest ${PROJECT_NAME})
