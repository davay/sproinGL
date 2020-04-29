add_library(GLXtras GLXtras.cpp)
add_library(Mesh Mesh.cpp)
target_include_directories(GLXtras PUBLIC ../glad .)
