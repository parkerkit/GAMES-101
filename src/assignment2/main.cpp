#include <Eigen/Core>
#include <cmath>
#include <iostream>
#include <numbers>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

#include "rasterizer.hpp"
#include "triangle.hpp"

Eigen::Matrix4f GetViewMatrix(Eigen::Vector3f eye_pos) {
  Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

  Eigen::Matrix4f translate;
  translate << 1, 0, 0, -eye_pos[0], 0, 1, 0, -eye_pos[1], 0, 0, 1, -eye_pos[2], 0, 0, 0, 1;

  view = translate * view;

  return view;
}

Eigen::Matrix4f GetModelMatrix([[maybe_unused]] float rotation_angle) {
  Eigen::Matrix4f model = Eigen::Matrix4f::Identity();
  return model;
}

Eigen::Matrix4f GetProjectionMatrix(float eye_fov, float aspect_ratio, float z_near, float z_far) {
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

int main(int argc, const char** argv) {
  float angle = 0;
  bool command_line = false;
  std::string filename = "output.png";

  if (argc == 2) {
    command_line = true;
    filename = std::string(argv[1]);
  }

  rst::Rasterizer r(700, 700);

  Eigen::Vector3f eye_pos = {0, 0, 5};

  std::vector<Eigen::Vector3f> pos{{2, 0, -2},     {0, 2, -2},       {-2, 0, -2},
                                   {3.5F, -1, -5}, {2.5F, 1.5F, -5}, {-1, 0.5F, -5}};

  std::vector<Eigen::Vector3i> ind{{0, 1, 2}, {3, 4, 5}};

  std::vector<Eigen::Vector3f> cols{{217.0F, 238.0F, 185.0F}, {217.0F, 238.0F, 185.0F}, {217.0F, 238.0F, 185.0F},
                                    {185.0F, 217.0F, 238.0F}, {185.0F, 217.0F, 238.0F}, {185.0F, 217.0F, 238.0F}};

  auto pos_id = r.LoadPositions(pos);
  auto ind_id = r.LoadIndices(ind);
  auto col_id = r.LoadColors(cols);

  int key = 0;
  int frame_count = 0;

  if (command_line) {
    r.Clear(rst::Buffers::Color | rst::Buffers::Depth);

    r.SetModel(GetModelMatrix(angle));
    r.SetView(GetViewMatrix(eye_pos));
    r.SetProjection(GetProjectionMatrix(45.0F, 1.0F, 0.1F, 50.0F));

    r.Draw(pos_id, ind_id, col_id, rst::Primitive::Triangle);
    cv::Mat image(700, 700, CV_32FC3, r.FrameBuffer().data());
    image.convertTo(image, CV_8UC3, 1.0F);
    cv::cvtColor(image, image, cv::COLOR_RGB2BGR);

    cv::imwrite(filename, image);

    return 0;
  }

  while (key != 27) {
    r.Clear(rst::Buffers::Color | rst::Buffers::Depth);

    r.SetModel(GetModelMatrix(angle));
    r.SetView(GetViewMatrix(eye_pos));
    r.SetProjection(GetProjectionMatrix(45.0F, 1.0F, 0.1F, 50.0F));

    r.Draw(pos_id, ind_id, col_id, rst::Primitive::Triangle);

    cv::Mat image(700, 700, CV_32FC3, r.FrameBuffer().data());
    image.convertTo(image, CV_8UC3, 1.0F);
    cv::cvtColor(image, image, cv::COLOR_RGB2BGR);
    cv::imshow("image", image);
    key = cv::waitKey(10);

    std::cout << "frame count: " << frame_count++ << '\n';
  }

  return 0;
}
