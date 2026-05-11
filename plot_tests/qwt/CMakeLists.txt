find_package(Qwt REQUIRED)
    
add_executable(qwt_example 
    window.cpp 
    window.h 
    main.cpp)
target_link_libraries(qwt_example qwt-qt6 Qt6::Widgets Qt6::OpenGL)