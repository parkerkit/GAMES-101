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

  float width = static_cast<float>(scene.GetWidth());
  float height = static_cast<float>(scene.GetHeight());

  // 成像平面距相机为 1，垂直半高为 tan(fov / 2)，水平半宽再乘宽高比。
  float scale = std::tan(DegToRad(scene.GetFov() * 0.5F));
  float image_aspect_ratio = static_cast<float>(scene.GetWidth()) / static_cast<float>(scene.GetHeight());
  Vector3f eye_pos(278, 273, -800);
  std::size_t pixel_index = 0;
  // 修改 spp 可调整每像素采样次数。
  constexpr int spp = 16;
  std::cout << "SPP: " << spp << '\n';
  UpdateProgress(0.0F);
  int last_progress_step = 0;
  for (int j = 0; j < scene.GetHeight(); ++j) {
    for (int i = 0; i < scene.GetWidth(); ++i) {
      // 加 0.5 取像素中心，再映射到 [-1, 1]；图像 Y 向下，相机 Y 向上，因此翻转 Y。
      float x = (2.0F * (static_cast<float>(i) + 0.5F) / width - 1.0F) * image_aspect_ratio * scale;
      float y = (1.0F - 2.0F * (static_cast<float>(j) + 0.5F) / height) * scale;

      // Cornell Box 的相机朝向正 Z 轴；翻转 X 以匹配课程框架的视角。
      Vector3f dir = Normalize(Vector3f(-x, y, 1.0F));
      for (int sample_index = 0; sample_index < spp; ++sample_index) {
        framebuffer[pixel_index] += scene.CastRay(Ray(eye_pos, dir), 0) / static_cast<float>(spp);
      }
      ++pixel_index;
    }
    // 每完成约 10% 输出一次日志，兼容不支持回车覆盖的 IDE 输出面板。
    const int progress_step = static_cast<int>(static_cast<float>(j + 1) / height * 10.0F);
    if (progress_step > last_progress_step) {
      UpdateProgress(static_cast<float>(progress_step) / 10.0F);
      last_progress_step = progress_step;
    }
  }

  std::cout << '\n';

  // P6 stores RGB bytes; no OpenCV dependency is needed for this assignment.
  std::ofstream output("binary.ppm", std::ios::binary);
  if (!output) {
    throw std::runtime_error("Failed to open binary.ppm for writing");
  }
  output << "P6\n" << scene.GetWidth() << ' ' << scene.GetHeight() << "\n255\n";
  // 沿用框架的 0.6 次幂输出变换。
  for (const auto& pixel : framebuffer) {
    const std::array<unsigned char, 3> color{static_cast<unsigned char>(255.0F * std::pow(Clamp(0, 1, pixel.x), 0.6F)),
                                             static_cast<unsigned char>(255.0F * std::pow(Clamp(0, 1, pixel.y), 0.6F)),
                                             static_cast<unsigned char>(255.0F * std::pow(Clamp(0, 1, pixel.z), 0.6F))};
    for (const unsigned char channel : color) {
      output.put(static_cast<char>(channel));
    }
  }
  output.close();
  if (!output) {
    throw std::runtime_error("Failed to write binary.ppm");
  }
}
