cmake_minimum_required(VERSION 3.20)
project(korzo_vonalzo)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenGL REQUIRED)

set (CMAKE_CXX_FLAGS "-O3")

add_executable(korzo_vonalzo Skeleton.cpp framework.cpp framework.h)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} opengl32 freeglut glew32)
endif(WIN32)

if(UNIX)
    target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} ${GLEW_LIBRARIES})
endif(UNIX)