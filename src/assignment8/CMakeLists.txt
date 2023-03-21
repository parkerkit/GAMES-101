set(target_name assignment8)

# Build the bundled FreeType without optional external font/compression libraries.
# These settings are local to Assignment 8 and its subdirectories.
set(BUILD_SHARED_LIBS OFF)
set(SKIP_INSTALL_ALL ON)
set(FT_DISABLE_ZLIB ON)
set(FT_DISABLE_BZIP2 ON)
set(FT_DISABLE_PNG ON)
set(FT_DISABLE_HARFBUZZ ON)
set(FT_DISABLE_BROTLI ON)
set(FT_DISABLE_HVF ON)
add_subdirectory("${PROJECT_SOURCE_DIR}/third_party/freetype" "${CMAKE_CURRENT_BINARY_DIR}/freetype" SYSTEM)
if(MSVC)
  # Apply UTF-8 only to C compilation; FreeType also contains a Windows RC source.
  target_compile_options(freetype PRIVATE $<$<COMPILE_LANGUAGE:C>:/utf-8>)
endif()
add_library(Freetype::Freetype ALIAS freetype-interface)

add_subdirectory("${PROJECT_SOURCE_DIR}/third_party/cgl" "${CMAKE_CURRENT_BINARY_DIR}/cgl")

add_executable(${target_name}
  main.cpp
  application.cpp
  rope.cpp
  application.hpp
  rope.hpp
  mass.hpp
  spring.hpp
)

target_link_libraries(${target_name} PRIVATE CGL::CGL)
project_setup_target(${target_name})
