#include <Eigen/Core>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

#include "rasterizer.hpp"
#include "triangle.hpp"

[[maybe_unused]] constexpr double k_pi = 3.1415926;

Eigen::Matrix4f GetViewMatrix(Eigen::Vector3f eye_pos) {
  Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

  Eigen::Matrix4f translate;
  translate << 1, 0, 0, -eye_pos[0], 0, 1, 0, -eye_pos[1], 0, 0, 1, -eye_pos[2], 0, 0, 0, 1;

  view = translate * view;

  return view;
}

Eigen::Matrix4f GetModelMatrix([[maybe_unused]] float rotation_angle) {
  Eigen::Matrix4f model = Eigen::Matrix4f::Identity();

  // TODO: Implement this function
  // Create the model matrix for rotating the triangle around the Z axis.
  // Then return it.

  return model;
}

Eigen::Matrix4f GetProjectionMatrix([[maybe_unused]] float eye_fov, [[maybe_unused]] float aspect_ratio,
                                    [[maybe_unused]] float z_near, [[maybe_unused]] float z_far) {
  // Students will implement this function

  Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();

  // TODO: Implement this function
  // Create the projection matrix for the given parameters.
  // Then return it.

  return projection;
}

int main(int argc, const char** argv) {
  float angle = 0;
  bool command_line = false;
  std::string filename = "output.png";

  if (argc >= 3) {
    command_line = true;
    angle = std::stof(argv[2]);  // -r by default
    if (argc == 4) {
      filename = std::string(argv[3]);
    } else {
      return 0;
    }
  }

  rst::Rasterizer r(700, 700);

  Eigen::Vector3f eye_pos = {0, 0, 5};

  std::vector<Eigen::Vector3f> pos{{2, 0, -2}, {0, 2, -2}, {-2, 0, -2}};

  std::vector<Eigen::Vector3i> ind{{0, 1, 2}};

  auto pos_id = r.LoadPositions(pos);
  auto ind_id = r.LoadIndices(ind);

  int key = 0;
  int frame_count = 0;

  if (command_line) {
    r.Clear(rst::Buffers::Color | rst::Buffers::Depth);

    r.SetModel(GetModelMatrix(angle));
    r.SetView(GetViewMatrix(eye_pos));
    r.SetProjection(GetProjectionMatrix(45.0F, 1.0F, 0.1F, 50.0F));

    r.Draw(pos_id, ind_id, rst::Primitive::Triangle);
    cv::Mat image(700, 700, CV_32FC3, r.FrameBuffer().data());
    image.convertTo(image, CV_8UC3, 1.0F);

    cv::imwrite(filename, image);

    return 0;
  }

  while (key != 27) {
    r.Clear(rst::Buffers::Color | rst::Buffers::Depth);

    r.SetModel(GetModelMatrix(angle));
    r.SetView(GetViewMatrix(eye_pos));
    r.SetProjection(GetProjectionMatrix(45.0F, 1.0F, 0.1F, 50.0F));

    r.Draw(pos_id, ind_id, rst::Primitive::Triangle);

    cv::Mat image(700, 700, CV_32FC3, r.FrameBuffer().data());
    image.convertTo(image, CV_8UC3, 1.0F);
    cv::imshow("image", image);
    key = cv::waitKey(10);

    std::cout << "frame count: " << frame_count++ << '\n';

    if (key == 'a') {
      angle += 10;
    } else if (key == 'd') {
      angle -= 10;
    }
  }

  return 0;
}
