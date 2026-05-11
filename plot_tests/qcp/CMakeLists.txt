# Turn on to enable rendering using OpenGL for both
add_definitions(-DQCUSTOMPLOT_USE_OPENGL)

add_executable(qcp_example 
    window.cpp 
    window.h 
    qcustomplot.cpp
    qcustomplot.h
    main.cpp)
target_link_libraries(qcp_example Qt6::Widgets Qt6::OpenGL Qt6::PrintSupport OpenGL::GL)
