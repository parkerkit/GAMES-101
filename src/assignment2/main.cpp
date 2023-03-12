#include <Eigen/Core>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <stdexcept>
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

Eigen::Matrix4f GetProjectionMatrix([[maybe_unused]] float eye_fov, [[maybe_unused]] float aspect_ratio,
                                    [[maybe_unused]] float z_near, [[maybe_unused]] float z_far) {
  // TODO: Copy-paste your implementation from the previous assignment.
  throw std::logic_error("TODO: Implement GetProjectionMatrix using Assignment 1.F");
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
