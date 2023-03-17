//
// Created by goksu on 2/25/20.
//

#include "renderer.hpp"

#include <array>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <stdexcept>

#include "global.hpp"
#include "scene.hpp"

namespace {
float DegToRad(float deg) { return deg * k_pi / 180.0F; }
}  // namespace

// The main render function. This where we iterate over all pixels in the image,
// generate primary rays and cast these rays into the scene. The content of the
// framebuffer is saved to a file.
void Renderer::Render(const Scene& scene) {
  std::vector<Vector3f> framebuffer(static_cast<std::size_t>(scene.GetWidth()) *
                                    static_cast<std::size_t>(scene.GetHeight()));

  float scale = std::tan(DegToRad(scene.GetFov() * 0.5F));
  float image_aspect_ratio = static_cast<float>(scene.GetWidth()) / static_cast<float>(scene.GetHeight());
  [[maybe_unused]] Vector3f eye_pos(-1, 5, 10);
  for (int j = 0; j < scene.GetHeight(); ++j) {
    for (int i = 0; i < scene.GetWidth(); ++i) {
      // generate primary ray direction
      [[maybe_unused]] float x =
        (2 * (static_cast<float>(i) + 0.5F) / static_cast<float>(scene.GetWidth()) - 1) * image_aspect_ratio * scale;
      [[maybe_unused]] float y =
        (1 - 2 * (static_cast<float>(j) + 0.5F) / static_cast<float>(scene.GetHeight())) * scale;
      // TODO: Build a normalized primary ray from eye_pos and (x, y, -1),
      // call scene.CastRay(ray, 0), and store the result in framebuffer.
    }
    UpdateProgress(static_cast<float>(j) / static_cast<float>(scene.GetHeight()));
  }
  UpdateProgress(1.F);

  std::cout << '\n';

  // P6 stores RGB bytes; no OpenCV dependency is needed for this assignment.
  std::ofstream output("binary.ppm", std::ios::binary);
  if (!output) {
    throw std::runtime_error("Failed to open binary.ppm for writing");
  }
  output << "P6\n" << scene.GetWidth() << ' ' << scene.GetHeight() << "\n255\n";
  for (const auto& pixel : framebuffer) {
    const std::array<unsigned char, 3> color{static_cast<unsigned char>(255.0F * Clamp(0, 1, pixel.x)),
                                             static_cast<unsigned char>(255.0F * Clamp(0, 1, pixel.y)),
                                             static_cast<unsigned char>(255.0F * Clamp(0, 1, pixel.z))};
    for (const unsigned char channel : color) {
      output.put(static_cast<char>(channel));
    }
  }
  output.close();
  if (!output) {
    throw std::runtime_error("Failed to write binary.ppm");
  }
}
