find_package(osg)
find_package(osgViewer)
find_package(osgDB)
find_package(osgUtil)
find_package(GLUT)

set(examples
    00_model_file
    01_simple_shapes
    02_quad
    03_octahedron
    04_tessellate
    05_triangle_face_collector
    06_custom_drawable
    07_translate
    08_lod
    09_load_at_runtime
    10_traverse
    11_visitor)

foreach(example_name ${examples})
    add_executable(${example_name} ${example_name}.cpp)
    target_include_directories(${example_name} PUBLIC ${GLUT_INCLUDE_DIRS})
    target_link_libraries(${example_name} osg osgUtil osgViewer osgDB glut GL)
endforeach()
