find_package(glfw3 3.3 REQUIRED)
find_package(Freetype REQUIRED)

add_library(bloomenthal Camera.cpp CameraArcball.cpp Color.cpp Draw.cpp GLXtras.cpp Lights.cpp Mesh.cpp Misc.cpp Numbers.cpp Polygonizer.cpp Quaternion.cpp Slider.cpp Sphere.cpp Widgets.cpp Text.cpp)
target_include_directories(bloomenthal PUBLIC ../glad ../GL . ${FREETYPE_INCLUDE_DIRS})
target_link_libraries(bloomenthal glfw ${FREETYPE_LIBRARIES})
