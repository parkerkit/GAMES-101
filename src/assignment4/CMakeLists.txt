set(target_name assignment4)

add_executable(${target_name} main.cpp)

target_include_directories(${target_name} SYSTEM PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(${target_name} PRIVATE ${OpenCV_LIBS})
project_setup_target(${target_name})
