# Original file Copyright CMU462 Fall 2015:
# Kayvon Fatahalian, Keenan Crane,
# Sky Gao, Bryce Summers, Michael Choquette.
# Build integration adapted for this repository; CGL sources retain their original layout.
cmake_minimum_required(VERSION 3.25)

find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)

# These settings are scoped to CGL and its bundled dependencies.
set(BUILD_SHARED_LIBS OFF)
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_INSTALL OFF)
add_subdirectory(deps/glfw)
target_include_directories(glfw SYSTEM PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/deps/glfw/include")
# The legacy GLFW script exports system dependencies separately for static linking.
target_link_libraries(glfw INTERFACE ${GLFW_LIBRARIES} Threads::Threads)

# Build only the GLEW library, without the glewinfo/visualinfo command-line programs.
add_library(glew STATIC deps/glew/src/glew.c)
target_include_directories(glew SYSTEM PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/deps/glew/include")
target_compile_definitions(glew PUBLIC GLEW_STATIC)
target_link_libraries(glew PUBLIC OpenGL::GL)

add_library(CGL STATIC
    src/vector2D.cpp
    src/vector3D.cpp
    src/vector4D.cpp
    src/matrix3x3.cpp
    src/matrix4x4.cpp
    src/quaternion.cpp
    src/complex.cpp
    src/color.cpp
    src/osdtext.cpp
    src/osdfont.c
    src/viewer.cpp
    src/base64.cpp
    src/tinyxml2.cpp
)
add_library(CGL::CGL ALIAS CGL)
target_include_directories(CGL SYSTEM PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(CGL PUBLIC glew glfw OpenGL::GL Freetype::Freetype Threads::Threads)
# Keep the third-party library on its original language standard.
set_target_properties(CGL PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON)
if(MSVC)
    target_compile_options(CGL PRIVATE /utf-8)
    target_compile_definitions(CGL PUBLIC _USE_MATH_DEFINES NOMINMAX)
    target_compile_definitions(CGL PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
