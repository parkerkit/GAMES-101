#include <chrono>
#include <exception>
#include <iostream>
#include <memory>
#include <string>

#include "global.hpp"
#include "renderer.hpp"
#include "scene.hpp"
#include "triangle.hpp"
#include "vector.hpp"

// In the main function of the program, we create the scene (create objects and
// lights) as well as set the options for the render (image width and height,
// maximum recursion depth, field-of-view, etc.). We then call the render
// function().
int main() {
  try {
    MeshTriangle bunny(std::string(ASSETS_PATH) + "models/bunny/bunny.obj");
    Scene scene(1280, 960);

    scene.Add(&bunny);
    scene.Add(std::make_unique<Light>(Vector3f(-20, 70, 20), Vector3f(1.0F)));
    scene.Add(std::make_unique<Light>(Vector3f(20, 70, 20), Vector3f(1.0F)));
    scene.BuildBVH();

    Renderer r;

    auto start = std::chrono::steady_clock::now();
    r.Render(scene);
    auto stop = std::chrono::steady_clock::now();

    std::cout << "Render complete: \n";
    std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::hours>(stop - start).count() << " hours\n";
    std::cout << "          : " << std::chrono::duration_cast<std::chrono::minutes>(stop - start).count()
              << " minutes\n";
    std::cout << "          : " << std::chrono::duration_cast<std::chrono::seconds>(stop - start).count()
              << " seconds\n";

    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
