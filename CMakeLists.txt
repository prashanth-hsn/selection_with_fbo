# CMakeList.txt : CMake project for mixed_mode_selection, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 20)

project ("select_with_fbo" LANGUAGES CXX C)

find_package(OpenGL REQUIRED)

include_directories( ${OPENGL_INCLUDE_DIRS} )

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

link_directories(${CMAKE_SOURCE_DIR}/3rdparty/glfw-3.3.9/bin)


# include_directories( 3rdparty/stb) 
include_directories( 3rdparty/glm-0.9.9.8)
include_directories( 3rdparty/glfw-3.3.9/include )
include_directories( 3rdparty/glad/include )

add_executable (select_with_fbo 
"src/main.cpp" 
"src/rubberband_glsl.h" 
"src/rubberband_glsl.cpp" 
"src/application.h" 
"src/application.cpp"
"src/cube_vbo.cpp"
"src/cube_vbo.h"
"src/basic_camera.cpp"
"src/basic_camera.h"
"3rdparty/glad/src/glad.c" )

target_link_libraries(select_with_fbo glfw3 ${OPENGL_LIBRARIES})


