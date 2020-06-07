cmake_minimum_required (VERSION 3.0)
project(sproinGL VERSION 0.01)
find_package(glfw3 3.3 REQUIRED)
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
find_package(Freetype REQUIRED)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_subdirectory(include)
add_executable(${PROJECT_NAME} src/main.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC include ../include/ ${FREETYPE_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} bloomenthal OpenGL::GL glfw GLAD ${CMAKE_DL_LIBS} ${FREETYPE_LIBRARIES})
