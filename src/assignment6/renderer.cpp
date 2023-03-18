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
  Vector3f eye_pos(-1, 5, 10);
  std::size_t pixel_index = 0;
  for (int j = 0; j < scene.GetHeight(); ++j) {
    for (int i = 0; i < scene.GetWidth(); ++i) {
      // 加 0.5 取像素中心，再映射到 [-1, 1]；图像 Y 向下，相机 Y 向上，因此翻转 Y。
      float x = (2.0F * (static_cast<float>(i) + 0.5F) / width - 1.0F) * image_aspect_ratio * scale;
      float y = (1.0F - 2.0F * (static_cast<float>(j) + 0.5F) / height) * scale;

      // 相机朝向负 Z 轴，方向是相对于相机的偏移，不需要减去世界空间的 eye_pos。
      // 归一化后，射线参数 t 可表示从光线起点出发的实际距离。
      Vector3f dir = Vector3f(x, y, -1.0F);
      dir = Normalize(dir);

      // 从相机位置发出主光线，递归深度从 0 开始，颜色按行写入帧缓冲。
      framebuffer[pixel_index++] = scene.CastRay({eye_pos, dir}, 0);
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
