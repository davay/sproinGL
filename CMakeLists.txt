cmake_minimum_required (VERSION 2.8.11)
project(sproinGL)
find_package(glfw3 3.3 REQUIRED)
find_package(OpenGL REQUIRED)

add_subdirectory(external)
add_executable(${PROJECT_NAME} src/main.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC include ../external/)
target_link_libraries(${PROJECT_NAME} OpenGL::GL glfw GLAD GLXtras ${CMAKE_DL_LIBS})

