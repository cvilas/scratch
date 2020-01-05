find_package(robohive COMPONENTS REQUIRED wemo)
find_package(osg)
find_package(osgViewer)
find_package(osgDB)
find_package(osgUtil)
find_package(Eigen3)
find_package(Threads)
find_package(foonathan_memory)

set(examples
    01_wemo)

foreach(example_name ${examples})
    add_executable(${example_name} ${example_name}.cpp)
    target_include_directories(${example_name} PUBLIC ${GLUT_INCLUDE_DIRS})
    target_link_libraries(${example_name} osg osgUtil osgViewer osgDB robohive::wemo)
endforeach()
