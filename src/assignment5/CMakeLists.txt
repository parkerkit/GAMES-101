set(target_name assignment5)

add_executable(${target_name}
  main.cpp
  renderer.cpp
  scene.cpp
  global.hpp
  light.hpp
  object.hpp
  renderer.hpp
  scene.hpp
  sphere.hpp
  triangle.hpp
  vector.hpp
)

project_setup_target(${target_name})
