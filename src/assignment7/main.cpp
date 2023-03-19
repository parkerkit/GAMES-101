#include <chrono>
#include <exception>
#include <iostream>
#include <string>

#include "material.hpp"
#include "renderer.hpp"
#include "scene.hpp"
#include "triangle.hpp"
#include "vector.hpp"

// Set up the Cornell Box scene and render it with the course framework.
int main() {
  try {
    Material red(Diffuse, Vector3f(0.0F));
    red.kd = Vector3f(0.63F, 0.065F, 0.05F);
    Material green(Diffuse, Vector3f(0.0F));
    green.kd = Vector3f(0.14F, 0.45F, 0.091F);
    Material white(Diffuse, Vector3f(0.0F));
    white.kd = Vector3f(0.725F, 0.71F, 0.68F);
    Material light(Diffuse, 8.0F * Vector3f(0.747F + 0.058F, 0.747F + 0.258F, 0.747F) +
                              15.6F * Vector3f(0.740F + 0.287F, 0.740F + 0.160F, 0.740F) +
                              18.4F * Vector3f(0.737F + 0.642F, 0.737F + 0.159F, 0.737F));
    light.kd = Vector3f(0.65F);

    const std::string model_path = std::string(ASSETS_PATH) + "models/cornellbox/";
    MeshTriangle floor(model_path + "floor.obj", &white);
    MeshTriangle short_box(model_path + "shortbox.obj", &white);
    MeshTriangle tall_box(model_path + "tallbox.obj", &white);
    MeshTriangle left(model_path + "left.obj", &red);
    MeshTriangle right(model_path + "right.obj", &green);
    MeshTriangle light_mesh(model_path + "light.obj", &light);

    // 场景只引用网格，网格只引用材质；按材质、网格、场景的顺序创建以保证生命周期。
    Scene scene(784, 784);
    scene.Add(&floor);
    scene.Add(&short_box);
    scene.Add(&tall_box);
    scene.Add(&left);
    scene.Add(&right);
    scene.Add(&light_mesh);
    scene.BuildBVH();

    Renderer renderer;
    auto start = std::chrono::steady_clock::now();
    renderer.Render(scene);
    auto stop = std::chrono::steady_clock::now();

    std::cout << "Render complete: \n";
    std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::seconds>(stop - start).count()
              << " seconds\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
