add_library(GLAD src/glad.c)
target_include_directories(GLAD PUBLIC include)
add_library(GLXtras src/GLXtras.cpp)
target_include_directories(GLXtras PUBLIC include)

