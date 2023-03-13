set(target_name assignment3)

add_executable(${target_name}
    main.cpp
    rasterizer.cpp
    rasterizer.hpp
    triangle.cpp
    triangle.hpp
    texture.hpp
    shader.hpp
)

target_include_directories(${target_name} SYSTEM PRIVATE
    ${OpenCV_INCLUDE_DIRS}
    "${PROJECT_SOURCE_DIR}/third_party/obj_loader"
)
target_link_libraries(${target_name} PRIVATE ${OpenCV_LIBS} Eigen3::Eigen)
project_setup_target(${target_name})
