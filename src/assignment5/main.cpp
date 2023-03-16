#include <array>
#include <cstdint>
#include <exception>
#include <iostream>
#include <memory>
#include <utility>

#include "light.hpp"
#include "renderer.hpp"
#include "scene.hpp"
#include "sphere.hpp"
#include "triangle.hpp"

// In the main function of the program, we create the scene (create objects and lights)
// as well as set the options for the render (image width and height, maximum recursion
// depth, field-of-view, etc.). We then call the render function().
int main() {
  try {
    Scene scene(1280, 960);

    auto sph1 = std::make_unique<Sphere>(Vector3f(-1, 0, -12), 2.0F);
    sph1->material_type = DiffuseAndGlossy;
    sph1->diffuse_color = Vector3f(0.6F, 0.7F, 0.8F);

    auto sph2 = std::make_unique<Sphere>(Vector3f(0.5F, -0.5F, -8), 1.5F);
    sph2->ior = 1.5F;
    sph2->material_type = ReflectionAndRefraction;

    scene.Add(std::move(sph1));
    scene.Add(std::move(sph2));

    const std::array<Vector3f, 4> verts = {Vector3f{-5, -3, -6}, Vector3f{5, -3, -6}, Vector3f{5, -3, -16},
                                           Vector3f{-5, -3, -16}};
    const std::array<uint32_t, 6> vert_index = {0, 1, 3, 1, 2, 3};
    const std::array<Vector2f, 4> st = {Vector2f{0, 0}, Vector2f{1, 0}, Vector2f{1, 1}, Vector2f{0, 1}};
    auto mesh = std::make_unique<MeshTriangle>(verts.data(), vert_index.data(), 2, st.data());
    mesh->material_type = DiffuseAndGlossy;

    scene.Add(std::move(mesh));
    scene.Add(std::make_unique<Light>(Vector3f(-20, 70, 20), 0.5F));
    scene.Add(std::make_unique<Light>(Vector3f(30, 50, -12), 0.5F));

    Renderer r;
    r.Render(scene);

    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
