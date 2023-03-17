set(target_name assignment6)

add_executable(${target_name}
  main.cpp
  bvh.cpp
  renderer.cpp
  scene.cpp
  vector.cpp
  area_light.hpp
  bounds3.hpp
  bvh.hpp
  global.hpp
  intersection.hpp
  light.hpp
  material.hpp
  object.hpp
  ray.hpp
  renderer.hpp
  scene.hpp
  sphere.hpp
  triangle.hpp
  vector.hpp
)

target_include_directories(${target_name} SYSTEM PRIVATE ${PROJECT_SOURCE_DIR}/third_party/obj_loader)
project_setup_target(${target_name})
