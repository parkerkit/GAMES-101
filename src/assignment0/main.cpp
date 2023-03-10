#include <Eigen/Core>
#include <iostream>
#include <opencv2/opencv.hpp>

void TestEigen() {
  std::cout << "------------------------------------------------\n";
  std::cout << "Eigen version: " << EIGEN_WORLD_VERSION << "." << EIGEN_MAJOR_VERSION << "." << EIGEN_MINOR_VERSION
            << "\n";

  // 创建 3D 向量（默认是列向量）
  Eigen::Vector3d point(1.0, 2.0, 3.0);
  Eigen::Vector3d direction(1.0, 0.0, 0.0);

  std::cout << "Point:\n" << point << "\n\n";

  // 向量基本操作
  std::cout << "Point + direction:\n" << point + direction << "\n\n";
  std::cout << "Dot product: " << point.dot(direction) << "\n";
  std::cout << "Norm: " << point.norm() << "\n\n";

  // 创建 3x3 旋转矩阵：绕 Z 轴旋转 90 度
  Eigen::Matrix3d rotate;
  rotate << 0.0, -1.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0;

  // 使用矩阵变换点
  Eigen::Vector3d rotated_point = rotate * point;
  std::cout << "Rotated point:\n" << rotated_point << "\n\n";

  // 平移变换
  Eigen::Vector3d translation(10.0, 0.0, 0.0);
  Eigen::Vector3d transformed_point = rotated_point + translation;

  std::cout << "Transformed point:\n" << transformed_point << "\n";
}

void TestOpenCV() {
  std::cout << "------------------------------------------------\n";
  std::cout << "OpenCV version: " << CV_VERSION << "\n";

  // 设置 OpenCV 日志级别
  cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_WARNING);

  // 创建一张 640x480 的黑色彩色图像
  cv::Mat image = cv::Mat::zeros(480, 640, CV_8UC3);

  // 绘制直线：绿色
  cv::line(image, cv::Point(50, 50), cv::Point(300, 50), cv::Scalar(0, 255, 0), 2);

  // 绘制矩形：蓝色
  cv::rectangle(image, cv::Point(50, 100), cv::Point(250, 250), cv::Scalar(255, 0, 0), 2);

  // 绘制圆形：红色
  cv::circle(image, cv::Point(400, 200), 60, cv::Scalar(0, 0, 255), 2);

  // 显示图像
  cv::imshow("OpenCV Basic", image);
  cv::waitKey(0);
}

int main() {
  TestEigen();
  TestOpenCV();
  return 0;
}
