#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <functional>
#include <iostream>
#include <memory>
#include <numbers>
#include <opencv2/opencv.hpp>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "OBJ_Loader.h"
#include "rasterizer.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "triangle.hpp"

Eigen::Matrix4f GetViewMatrix(Eigen::Vector3f eye_pos) {
  Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

  Eigen::Matrix4f translate;
  translate << 1, 0, 0, -eye_pos[0], 0, 1, 0, -eye_pos[1], 0, 0, 1, -eye_pos[2], 0, 0, 0, 1;

  view = translate * view;

  return view;
}

Eigen::Matrix4f GetModelMatrix(float angle) {
  Eigen::Matrix4f rotation;
  angle = angle * std::numbers::pi_v<float> / 180.F;
  rotation << std::cos(angle), 0, std::sin(angle), 0, 0, 1, 0, 0, -std::sin(angle), 0, std::cos(angle), 0, 0, 0, 0, 1;

  Eigen::Matrix4f scale;
  scale << 2.5F, 0, 0, 0, 0, 2.5F, 0, 0, 0, 0, 2.5F, 0, 0, 0, 0, 1;

  Eigen::Matrix4f translate;
  translate << 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1;

  return translate * rotation * scale;
}

Eigen::Matrix4f GetProjectionMatrix(float eye_fov, float aspect_ratio, float z_near, float z_far) {
  if (!std::isfinite(eye_fov) || !std::isfinite(aspect_ratio) || !std::isfinite(z_near) || !std::isfinite(z_far) ||
      eye_fov <= 0.0F || eye_fov >= 180.0F || aspect_ratio <= 0.0F || z_near <= 0.0F || z_far <= z_near) {
    throw std::invalid_argument("Invalid perspective projection parameters");
  }
  // 相机朝向 -Z；z_near、z_far 是正距离，近平面和远平面的相机空间坐标均为负。
  const float near_z = -z_near;
  const float far_z = -z_far;

  // 将视锥体压缩为长方体；取 w = -z，使相机前方顶点的齐次 w 为正。
  Eigen::Matrix4f perspective_transform{};
  // clang-format off
  perspective_transform << -near_z, 0,       0,                 0,
                           0,       -near_z, 0,                 0,
                           0,       0,       -(near_z + far_z), near_z * far_z,
                           0,       0,       -1,                0;
  // clang-format on

  const float half_eye_radian = eye_fov * std::numbers::pi_v<float> / 360.0F;
  const float top = z_near * std::tan(half_eye_radian);
  const float bottom = -top;
  const float right = top * aspect_ratio;
  const float left = -right;

  // 将长方体平移到原点，再缩放至 NDC 的 [-1, 1] 范围。
  Eigen::Matrix4f ndc_scale{};
  Eigen::Matrix4f frustum_center_translation{};
  // clang-format off
  ndc_scale << 2 / (right - left), 0,                  0,                    0,
               0,                  2 / (top - bottom), 0,                    0,
               0,                  0,                  2 / (near_z - far_z), 0,
               0,                  0,                  0,                    1;

  frustum_center_translation << 1, 0, 0, -(right + left) / 2,
                                0, 1, 0, -(top + bottom) / 2,
                                0, 0, 1, -(near_z + far_z) / 2,
                                0, 0, 0, 1;
  // clang-format on

  // 透视除法后：近平面 NDC z = +1，远平面 NDC z = -1。
  // 与光栅化阶段保持一致：深度越大越靠近相机。
  return ndc_scale * frustum_center_translation * perspective_transform;
}

Eigen::Vector3f VertexShader(const VertexShaderPayload& payload) { return payload.position; }

namespace {

constexpr float k_direction_epsilon = 1e-12F;
constexpr float k_height_scale = 0.2F;
constexpr float k_displacement_scale = 0.1F;

Eigen::Vector3f NormalizeNormal(const Eigen::Vector3f& normal) {
  if (!normal.allFinite() || normal.squaredNorm() <= k_direction_epsilon) {
    throw std::invalid_argument("Surface normal must be finite and nonzero");
  }
  return normal.normalized();
}

Eigen::Vector3f NormalToColor(const Eigen::Vector3f& normal) {
  // 将法线分量从 [-1, 1] 映射到 [0, 255]，保留负分量对应的信息。
  return (NormalizeNormal(normal) + Eigen::Vector3f::Ones()) * 127.5F;
}

struct Light {
  Eigen::Vector3f position;
  Eigen::Vector3f intensity;
};

Eigen::Vector3f EvaluateLighting(const Eigen::Vector3f& point, const Eigen::Vector3f& surface_normal,
                                 const Eigen::Vector3f& kd) {
  const Eigen::Vector3f ka(0.005F, 0.005F, 0.005F);
  const Eigen::Vector3f ks(0.7937F, 0.7937F, 0.7937F);
  const Eigen::Vector3f amb_light_intensity(10, 10, 10);
  constexpr float shininess = 150.0F;

  // 灯光、片元位置和法线均定义在相机空间；相机位于原点。
  const std::array<Light, 2> lights{Light{{20, 20, 20}, {500, 500, 500}}, Light{{-20, 20, 0}, {500, 500, 500}}};
  const Eigen::Vector3f normal = NormalizeNormal(surface_normal);
  const bool has_view_direction = point.squaredNorm() > k_direction_epsilon;
  const Eigen::Vector3f view_dir = (-point).normalized();

  // 环境光按通道相乘，只计算一次；各点光源分别贡献漫反射和高光。
  Eigen::Vector3f result_color = ka.cwiseProduct(amb_light_intensity);
  for (const auto& light : lights) {
    const Eigen::Vector3f to_light = light.position - point;
    const float distance_squared = to_light.squaredNorm();
    if (distance_squared <= k_direction_epsilon) {
      continue;
    }
    const Eigen::Vector3f light_dir = to_light.normalized();
    const Eigen::Vector3f intensity = light.intensity / distance_squared;
    const float diffuse_factor = std::max(0.0F, normal.dot(light_dir));
    if (diffuse_factor <= 0.0F) {
      continue;
    }
    result_color += kd.cwiseProduct(intensity) * diffuse_factor;

    // Blinn–Phong 使用半程向量；光线与视线恰好反向时跳过高光。
    const Eigen::Vector3f half_vector = view_dir + light_dir;
    if (has_view_direction && half_vector.squaredNorm() > k_direction_epsilon) {
      const float specular_factor = std::pow(std::max(0.0F, normal.dot(half_vector.normalized())), shininess);
      result_color += ks.cwiseProduct(intensity) * specular_factor;
    }
  }
  return result_color * 255.0F;
}

float SampleHeight(const Texture& texture, float u, float v) {
  // RGB 向量长度归一化到 [0, 1]，避免把 0–255 色值直接作为几何位移。
  return texture.GetColor(u, v).norm() / (255.0F * std::sqrt(3.0F));
}

Eigen::Vector3f PerturbNormal(const FragmentShaderPayload& payload) {
  if (payload.texture == nullptr) {
    throw std::invalid_argument("Bump and displacement shaders require a height map");
  }
  const Texture& texture = *payload.texture;
  const Eigen::Vector3f normal = NormalizeNormal(payload.normal);

  // 构造正交的切线、副切线、法线基底；法线平行 Y 轴时单独选择切线，避免除零。
  const float xz_length = std::hypot(normal.x(), normal.z());
  Eigen::Vector3f tangent = Eigen::Vector3f::UnitX();
  if (xz_length > 1e-6F) {
    tangent = Eigen::Vector3f(normal.x() * normal.y() / xz_length, -xz_length, normal.z() * normal.y() / xz_length);
    tangent.normalize();
  }
  const Eigen::Vector3f bitangent = normal.cross(tangent).normalized();
  Eigen::Matrix3f tbn;
  tbn.col(0) = tangent;
  tbn.col(1) = bitangent;
  tbn.col(2) = normal;

  if (!payload.tex_coords.allFinite()) {
    throw std::invalid_argument("Texture coordinates must be finite");
  }
  const float u = std::clamp(payload.tex_coords.x(), 0.0F, 1.0F);
  const float v = std::clamp(payload.tex_coords.y(), 0.0F, 1.0F);
  const float step_u = 1.0F / static_cast<float>(texture.GetWidth());
  const float step_v = 1.0F / static_cast<float>(texture.GetHeight());

  // 用相邻纹素估计高度对 UV 的导数；边界处改用向后差分。
  const float u0 = u + step_u <= 1.0F ? u : std::max(0.0F, u - step_u);
  const float u1 = std::min(1.0F, u0 + step_u);
  const float v0 = v + step_v <= 1.0F ? v : std::max(0.0F, v - step_v);
  const float v1 = std::min(1.0F, v0 + step_v);
  const float du =
    k_height_scale * k_displacement_scale * (SampleHeight(texture, u1, v) - SampleHeight(texture, u0, v)) / (u1 - u0);
  const float dv =
    k_height_scale * k_displacement_scale * (SampleHeight(texture, u, v1) - SampleHeight(texture, u, v0)) / (v1 - v0);

  // 高度沿切线方向增加时，表面法线向反方向偏转，再由 TBN 转回相机空间。
  return NormalizeNormal(tbn * Eigen::Vector3f(-du, -dv, 1.0F));
}

}  // namespace

Eigen::Vector3f NormalFragmentShader(const FragmentShaderPayload& payload) { return NormalToColor(payload.normal); }

Eigen::Vector3f PhongFragmentShader(const FragmentShaderPayload& payload) {
  return EvaluateLighting(payload.view_pos, payload.normal, payload.color);
}

Eigen::Vector3f TextureFragmentShader(const FragmentShaderPayload& payload) {
  if (payload.texture == nullptr) {
    throw std::invalid_argument("Texture shader requires a color texture");
  }
  // 纹理作为漫反射系数，归一化后复用相同的光照模型。
  const Eigen::Vector3f kd = payload.texture->GetColor(payload.tex_coords.x(), payload.tex_coords.y()) / 255.0F;
  return EvaluateLighting(payload.view_pos, payload.normal, kd);
}

Eigen::Vector3f DisplacementFragmentShader(const FragmentShaderPayload& payload) {
  const Eigen::Vector3f perturbed_normal = PerturbNormal(payload);
  const Eigen::Vector3f normal = NormalizeNormal(payload.normal);
  const float height = SampleHeight(*payload.texture, payload.tex_coords.x(), payload.tex_coords.y());
  // 按课程片元着色框架偏移着色位置；这里不会改变网格轮廓或已经计算的深度。
  const Eigen::Vector3f displaced_point = payload.view_pos + k_displacement_scale * height * normal;
  return EvaluateLighting(displaced_point, perturbed_normal, payload.color);
}

Eigen::Vector3f BumpFragmentShader(const FragmentShaderPayload& payload) {
  // 凹凸映射只扰动法线，使用颜色显示法线方向，便于观察高度图效果。
  return NormalToColor(PerturbNormal(payload));
}

int main(int argc, const char** argv) {
  try {
    if (argc > 3) {
      std::cerr << "Usage: assignment3 [output.png [normal|phong|texture|bump|displacement]]\n";
      return 1;
    }
    const bool command_line = argc >= 2;
    const std::string filename = command_line ? argv[1] : "output.png";
    if (filename.empty()) {
      throw std::invalid_argument("Output filename must not be empty");
    }

    const std::string shader_mode = argc == 3 ? argv[2] : "bump";
    std::function<Eigen::Vector3f(FragmentShaderPayload)> active_shader;
    if (shader_mode == "normal") {
      active_shader = NormalFragmentShader;
    } else if (shader_mode == "phong") {
      active_shader = PhongFragmentShader;
    } else if (shader_mode == "texture") {
      active_shader = TextureFragmentShader;
    } else if (shader_mode == "bump") {
      active_shader = BumpFragmentShader;
    } else if (shader_mode == "displacement") {
      active_shader = DisplacementFragmentShader;
    } else {
      throw std::invalid_argument("Unknown shader: " + shader_mode);
    }
    std::cout << "Rasterizing using the " << shader_mode << " shader\n";

    std::vector<std::unique_ptr<Triangle>> triangles;
    std::vector<Triangle*> triangle_list;
    objl::Loader loader;
    const std::string obj_path = std::string(ASSETS_PATH) + "models/spot/";
    if (!loader.LoadFile(obj_path + "spot_triangulated_good.obj")) {
      throw std::runtime_error("Failed to load model: " + obj_path + "spot_triangulated_good.obj");
    }
    // 使用加载器生成的三角形索引，避免假设 OBJ 顶点一定按每三个组成一个面排列。
    for (const auto& mesh : loader.LoadedMeshes) {
      if (mesh.Indices.size() % 3 != 0) {
        throw std::runtime_error("Model contains an incomplete triangle");
      }
      for (std::size_t i = 0; i < mesh.Indices.size(); i += 3) {
        auto triangle = std::make_unique<Triangle>();
        for (int j = 0; j < 3; ++j) {
          const auto index = mesh.Indices[i + static_cast<std::size_t>(j)];
          const auto& vertex = mesh.Vertices.at(index);
          triangle->SetVertex(j, Eigen::Vector4f(vertex.Position.X, vertex.Position.Y, vertex.Position.Z, 1.0F));
          triangle->SetNormal(j, Eigen::Vector3f(vertex.Normal.X, vertex.Normal.Y, vertex.Normal.Z));
          triangle->SetTexCoord(j, Eigen::Vector2f(vertex.TextureCoordinate.X, vertex.TextureCoordinate.Y));
        }
        triangle_list.push_back(triangle.get());
        triangles.push_back(std::move(triangle));
      }
    }
    if (triangle_list.empty()) {
      throw std::runtime_error("Model contains no triangles");
    }

    constexpr int width = 700;
    constexpr int height = 700;
    rst::Rasterizer rasterizer(width, height);
    // 法线和纯色光照模式不依赖纹理文件；其余模式按需加载颜色图或高度图。
    if (shader_mode == "texture") {
      rasterizer.SetTexture(Texture(obj_path + "spot_texture.png"));
    } else if (shader_mode == "bump" || shader_mode == "displacement") {
      rasterizer.SetTexture(Texture(obj_path + "hmap.jpg"));
    }
    rasterizer.SetVertexShader(VertexShader);
    rasterizer.SetFragmentShader(std::move(active_shader));
    rasterizer.SetView(GetViewMatrix(Eigen::Vector3f(0, 0, 10)));
    rasterizer.SetProjection(
      GetProjectionMatrix(45.0F, static_cast<float>(width) / static_cast<float>(height), 0.1F, 50.0F));

    float angle = 140.0F;
    // 两种运行模式共用相同的绘制与 RGB/BGR 转换流程。
    const auto render_frame = [&]() -> cv::Mat {
      rasterizer.Clear(rst::Buffers::Color | rst::Buffers::Depth);
      rasterizer.SetModel(GetModelMatrix(angle));
      rasterizer.Draw(triangle_list);
      cv::Mat frame(height, width, CV_32FC3, rasterizer.FrameBuffer().data());
      cv::Mat image;
      frame.convertTo(image, CV_8UC3);
      cv::cvtColor(image, image, cv::COLOR_RGB2BGR);
      return image;
    };

    if (command_line) {
      if (!cv::imwrite(filename, render_frame())) {
        throw std::runtime_error("Failed to save image: " + filename);
      }
      return 0;
    }

    while (true) {
      cv::imshow("image", render_frame());
      const int key = cv::waitKey(10);
      if (key == 27) {
        break;
      }
      if (key == 'a') {
        angle -= 5.0F;
      } else if (key == 'd') {
        angle += 5.0F;
      }
      angle = std::remainder(angle, 360.0F);
    }
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << "\n";
    return 1;
  }
}
