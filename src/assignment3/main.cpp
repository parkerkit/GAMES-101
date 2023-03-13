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

Eigen::Matrix4f GetProjectionMatrix([[maybe_unused]] float eye_fov, [[maybe_unused]] float aspect_ratio,
                                    [[maybe_unused]] float z_near, [[maybe_unused]] float z_far) {
  // TODO: Use the same projection matrix from the previous assignments
  throw std::logic_error("TODO: implement GetProjectionMatrix before rendering Assignment 3");
}

Eigen::Vector3f VertexShader(const VertexShaderPayload& payload) { return payload.position; }

Eigen::Vector3f NormalFragmentShader(const FragmentShaderPayload& payload) {
  Eigen::Vector3f return_color = (payload.normal.head<3>().normalized() + Eigen::Vector3f(1.0F, 1.0F, 1.0F)) / 2.F;
  Eigen::Vector3f result;
  result << return_color.x() * 255, return_color.y() * 255, return_color.z() * 255;
  return result;
}

[[maybe_unused]] static Eigen::Vector3f Reflect(const Eigen::Vector3f& vec, const Eigen::Vector3f& axis) {
  auto costheta = vec.dot(axis);
  return (2 * costheta * axis - vec).normalized();
}

struct Light {
  Eigen::Vector3f position;
  Eigen::Vector3f intensity;
};

Eigen::Vector3f TextureFragmentShader(const FragmentShaderPayload& payload) {
  Eigen::Vector3f return_color = {0, 0, 0};
  if (payload.texture) {
    // TODO: Get the texture value at the texture coordinates of the current fragment
  }
  Eigen::Vector3f texture_color;
  texture_color << return_color.x(), return_color.y(), return_color.z();

  [[maybe_unused]] Eigen::Vector3f ka = Eigen::Vector3f(0.005F, 0.005F, 0.005F);
  [[maybe_unused]] Eigen::Vector3f kd = texture_color / 255.F;
  [[maybe_unused]] Eigen::Vector3f ks = Eigen::Vector3f(0.7937F, 0.7937F, 0.7937F);

  auto l1 = Light{{20, 20, 20}, {500, 500, 500}};
  auto l2 = Light{{-20, 20, 0}, {500, 500, 500}};

  std::vector<Light> lights = {l1, l2};
  [[maybe_unused]] Eigen::Vector3f amb_light_intensity{10, 10, 10};
  [[maybe_unused]] Eigen::Vector3f eye_pos{0, 0, 10};

  [[maybe_unused]] float p = 150;

  [[maybe_unused]] Eigen::Vector3f color = texture_color;
  [[maybe_unused]] Eigen::Vector3f point = payload.view_pos;
  [[maybe_unused]] Eigen::Vector3f normal = payload.normal;

  Eigen::Vector3f result_color = {0, 0, 0};

  for ([[maybe_unused]] const auto& light : lights) {
    // TODO: For each light source in the code, calculate what the *ambient*, *diffuse*, and *specular*
    // components are. Then, accumulate that result on the *result_color* object.
  }

  return result_color * 255.F;
}

Eigen::Vector3f PhongFragmentShader(const FragmentShaderPayload& payload) {
  [[maybe_unused]] Eigen::Vector3f ka = Eigen::Vector3f(0.005F, 0.005F, 0.005F);
  [[maybe_unused]] Eigen::Vector3f kd = payload.color;
  [[maybe_unused]] Eigen::Vector3f ks = Eigen::Vector3f(0.7937F, 0.7937F, 0.7937F);

  auto l1 = Light{{20, 20, 20}, {500, 500, 500}};
  auto l2 = Light{{-20, 20, 0}, {500, 500, 500}};

  std::vector<Light> lights = {l1, l2};
  [[maybe_unused]] Eigen::Vector3f amb_light_intensity{10, 10, 10};
  [[maybe_unused]] Eigen::Vector3f eye_pos{0, 0, 10};

  [[maybe_unused]] float p = 150;

  [[maybe_unused]] Eigen::Vector3f color = payload.color;
  [[maybe_unused]] Eigen::Vector3f point = payload.view_pos;
  [[maybe_unused]] Eigen::Vector3f normal = payload.normal;

  Eigen::Vector3f result_color = {0, 0, 0};
  for ([[maybe_unused]] const auto& light : lights) {
    // TODO: For each light source in the code, calculate what the *ambient*, *diffuse*, and *specular*
    // components are. Then, accumulate that result on the *result_color* object.
  }

  return result_color * 255.F;
}

Eigen::Vector3f DisplacementFragmentShader(const FragmentShaderPayload& payload) {
  [[maybe_unused]] Eigen::Vector3f ka = Eigen::Vector3f(0.005F, 0.005F, 0.005F);
  [[maybe_unused]] Eigen::Vector3f kd = payload.color;
  [[maybe_unused]] Eigen::Vector3f ks = Eigen::Vector3f(0.7937F, 0.7937F, 0.7937F);

  auto l1 = Light{{20, 20, 20}, {500, 500, 500}};
  auto l2 = Light{{-20, 20, 0}, {500, 500, 500}};

  std::vector<Light> lights = {l1, l2};
  [[maybe_unused]] Eigen::Vector3f amb_light_intensity{10, 10, 10};
  [[maybe_unused]] Eigen::Vector3f eye_pos{0, 0, 10};

  [[maybe_unused]] float p = 150;

  [[maybe_unused]] Eigen::Vector3f color = payload.color;
  [[maybe_unused]] Eigen::Vector3f point = payload.view_pos;
  [[maybe_unused]] Eigen::Vector3f normal = payload.normal;

  [[maybe_unused]] float kh = 0.2F;
  [[maybe_unused]] float kn = 0.1F;

  // TODO: Implement displacement mapping here
  // Let n = normal = (x, y, z)
  // Vector t = (x*y/sqrt(x*x+z*z),sqrt(x*x+z*z),z*y/sqrt(x*x+z*z))
  // Vector b = n cross product t
  // Matrix TBN = [t b n]
  // dU = kh * kn * (h(u+1/w,v)-h(u,v))
  // dV = kh * kn * (h(u,v+1/h)-h(u,v))
  // Vector ln = (-dU, -dV, 1)
  // Position p = p + kn * n * h(u,v)
  // Normal n = normalize(TBN * ln)

  Eigen::Vector3f result_color = {0, 0, 0};

  for ([[maybe_unused]] const auto& light : lights) {
    // TODO: For each light source in the code, calculate what the *ambient*, *diffuse*, and *specular*
    // components are. Then, accumulate that result on the *result_color* object.
  }

  return result_color * 255.F;
}

Eigen::Vector3f BumpFragmentShader(const FragmentShaderPayload& payload) {
  [[maybe_unused]] Eigen::Vector3f ka = Eigen::Vector3f(0.005F, 0.005F, 0.005F);
  [[maybe_unused]] Eigen::Vector3f kd = payload.color;
  [[maybe_unused]] Eigen::Vector3f ks = Eigen::Vector3f(0.7937F, 0.7937F, 0.7937F);

  auto l1 = Light{{20, 20, 20}, {500, 500, 500}};
  auto l2 = Light{{-20, 20, 0}, {500, 500, 500}};

  std::vector<Light> lights = {l1, l2};
  [[maybe_unused]] Eigen::Vector3f amb_light_intensity{10, 10, 10};
  [[maybe_unused]] Eigen::Vector3f eye_pos{0, 0, 10};

  [[maybe_unused]] float p = 150;

  [[maybe_unused]] Eigen::Vector3f color = payload.color;
  [[maybe_unused]] Eigen::Vector3f point = payload.view_pos;
  Eigen::Vector3f normal = payload.normal;

  [[maybe_unused]] float kh = 0.2F;
  [[maybe_unused]] float kn = 0.1F;

  // TODO: Implement bump mapping here
  // Let n = normal = (x, y, z)
  // Vector t = (x*y/sqrt(x*x+z*z),sqrt(x*x+z*z),z*y/sqrt(x*x+z*z))
  // Vector b = n cross product t
  // Matrix TBN = [t b n]
  // dU = kh * kn * (h(u+1/w,v)-h(u,v))
  // dV = kh * kn * (h(u,v+1/h)-h(u,v))
  // Vector ln = (-dU, -dV, 1)
  // Normal n = normalize(TBN * ln)

  Eigen::Vector3f result_color = {0, 0, 0};
  result_color = normal;

  return result_color * 255.F;
}

int main(int argc, const char** argv) {
  try {
    if (argc > 3) {
      std::cerr << "Usage: assignment3 [output.png [normal|phong|texture|bump|displacement]]\n";
      return 1;
    }
    std::vector<std::unique_ptr<Triangle>> triangles;
    std::vector<Triangle*> triangle_list;

    float angle = 140.0F;
    bool command_line = false;

    std::string filename = "output.png";
    objl::Loader loader;
    const std::string obj_path = std::string(ASSETS_PATH) + "models/spot/";

    // Load .obj File
    if (!loader.LoadFile(obj_path + "spot_triangulated_good.obj")) {
      throw std::runtime_error("Failed to load model: " + obj_path + "spot_triangulated_good.obj");
    }
    for (const auto& mesh : loader.LoadedMeshes) {
      for (std::size_t i = 0; i + 2 < mesh.Vertices.size(); i += 3) {
        auto t = std::make_unique<Triangle>();
        for (int j = 0; j < 3; ++j) {
          const auto& vertex = mesh.Vertices[i + static_cast<std::size_t>(j)];
          t->SetVertex(j, Eigen::Vector4f(vertex.Position.X, vertex.Position.Y, vertex.Position.Z, 1.0F));
          t->SetNormal(j, Eigen::Vector3f(vertex.Normal.X, vertex.Normal.Y, vertex.Normal.Z));
          t->SetTexCoord(j, Eigen::Vector2f(vertex.TextureCoordinate.X, vertex.TextureCoordinate.Y));
        }
        triangle_list.push_back(t.get());
        triangles.push_back(std::move(t));
      }
    }

    if (triangle_list.empty()) {
      throw std::runtime_error("Model contains no triangles");
    }

    rst::Rasterizer r(700, 700);

    auto texture_path = "hmap.jpg";
    r.SetTexture(Texture(obj_path + texture_path));

    std::function<Eigen::Vector3f(FragmentShaderPayload)> active_shader = PhongFragmentShader;

    if (argc >= 2) {
      command_line = true;
      filename = std::string(argv[1]);

      if (argc == 3 && std::string(argv[2]) == "texture") {
        std::cout << "Rasterizing using the texture shader\n";
        active_shader = TextureFragmentShader;
        texture_path = "spot_texture.png";
        r.SetTexture(Texture(obj_path + texture_path));
      } else if (argc == 3 && std::string(argv[2]) == "normal") {
        std::cout << "Rasterizing using the normal shader\n";
        active_shader = NormalFragmentShader;
      } else if (argc == 3 && std::string(argv[2]) == "phong") {
        std::cout << "Rasterizing using the phong shader\n";
        active_shader = PhongFragmentShader;
      } else if (argc == 3 && std::string(argv[2]) == "bump") {
        std::cout << "Rasterizing using the bump shader\n";
        active_shader = BumpFragmentShader;
      } else if (argc == 3 && std::string(argv[2]) == "displacement") {
        std::cout << "Rasterizing using the displacement shader\n";
        active_shader = DisplacementFragmentShader;
      } else if (argc == 3) {
        throw std::invalid_argument("Unknown shader: " + std::string(argv[2]));
      }
    }

    Eigen::Vector3f eye_pos = {0, 0, 10};

    r.SetVertexShader(VertexShader);
    r.SetFragmentShader(active_shader);

    int key = 0;

    if (command_line) {
      r.Clear(rst::Buffers::Color | rst::Buffers::Depth);
      r.SetModel(GetModelMatrix(angle));
      r.SetView(GetViewMatrix(eye_pos));
      r.SetProjection(GetProjectionMatrix(45.0F, 1, 0.1F, 50));

      r.Draw(triangle_list);
      cv::Mat image(700, 700, CV_32FC3, r.FrameBuffer().data());
      image.convertTo(image, CV_8UC3, 1.0F);
      cv::cvtColor(image, image, cv::COLOR_RGB2BGR);

      if (!cv::imwrite(filename, image)) {
        throw std::runtime_error("Failed to save image: " + filename);
      }

      return 0;
    }

    while (key != 27) {
      r.Clear(rst::Buffers::Color | rst::Buffers::Depth);

      r.SetModel(GetModelMatrix(angle));
      r.SetView(GetViewMatrix(eye_pos));
      r.SetProjection(GetProjectionMatrix(45.0F, 1, 0.1F, 50));

      r.Draw(triangle_list);
      cv::Mat image(700, 700, CV_32FC3, r.FrameBuffer().data());
      image.convertTo(image, CV_8UC3, 1.0F);
      cv::cvtColor(image, image, cv::COLOR_RGB2BGR);

      cv::imshow("image", image);
      key = cv::waitKey(10);

      if (key == 'a') {
        angle -= 0.1F;
      } else if (key == 'd') {
        angle += 0.1F;
      }
    }
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << "\n";
    return 1;
  }
}
