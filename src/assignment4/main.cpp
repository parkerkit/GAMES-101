#include <cmath>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <stdexcept>
#include <vector>

void MouseHandler(int event, int x, int y, [[maybe_unused]] int flags, void* user_data) {
  if (event != cv::EVENT_LBUTTONDOWN || user_data == nullptr) {
    return;
  }
  auto& control_points = *static_cast<std::vector<cv::Point2f>*>(user_data);
  if (control_points.size() < 4) {
    std::cout << "Left button of the mouse is clicked - position (" << x << ", " << y << ")\n";
    control_points.emplace_back(static_cast<float>(x), static_cast<float>(y));
  }
}

void NaiveBezier(const std::vector<cv::Point2f>& points, cv::Mat& window) {
  if (points.size() != 4) {
    throw std::invalid_argument("Cubic Bezier curve requires four control points");
  }
  const auto& p0 = points[0];
  const auto& p1 = points[1];
  const auto& p2 = points[2];
  const auto& p3 = points[3];

  // 原框架提供的三次 Bernstein 多项式公式；整数步进保证包含 t = 0 和 t = 1。
  constexpr int steps = 1000;
  for (int step = 0; step <= steps; ++step) {
    const float t = static_cast<float>(step) / static_cast<float>(steps);
    const float one_minus_t = 1.0F - t;
    const cv::Point2f point = one_minus_t * one_minus_t * one_minus_t * p0 + 3.0F * t * one_minus_t * one_minus_t * p1 +
                              3.0F * t * t * one_minus_t * p2 + t * t * t * p3;
    const int x = static_cast<int>(std::lround(point.x));
    const int y = static_cast<int>(std::lround(point.y));
    if (x >= 0 && x < window.cols && y >= 0 && y < window.rows) {
      // OpenCV 使用 BGR 顺序，通道 2 对应红色。
      window.at<cv::Vec3b>(y, x)[2] = 255;
    }
  }
}

cv::Point2f RecursiveBezier([[maybe_unused]] const std::vector<cv::Point2f>& control_points, [[maybe_unused]] float t) {
  // TODO: 实现 de Casteljau 算法，递归插值控制点，返回参数 t 对应的曲线点。
  throw std::logic_error("TODO: implement RecursiveBezier");
}

void Bezier([[maybe_unused]] const std::vector<cv::Point2f>& control_points, [[maybe_unused]] cv::Mat& window) {
  // TODO: 遍历 t = 0 到 t = 1，调用 RecursiveBezier，并将曲线绘制到 window。
  throw std::logic_error("TODO: implement Bezier");
}

int main() {
  try {
    std::vector<cv::Point2f> control_points;
    cv::Mat window(700, 700, CV_8UC3, cv::Scalar::all(0));
    cv::namedWindow("Bezier Curve", cv::WINDOW_AUTOSIZE);
    cv::setMouseCallback("Bezier Curve", MouseHandler, &control_points);

    while (true) {
      for (const auto& point : control_points) {
        cv::circle(window, cv::Point(static_cast<int>(point.x), static_cast<int>(point.y)), 3,
                   cv::Scalar(255, 255, 255), 3);
      }

      if (control_points.size() == 4) {
        NaiveBezier(control_points, window);
        // 完成 TODO 后启用，与直接公式绘制的曲线进行比较。
        // Bezier(control_points, window);
        cv::imshow("Bezier Curve", window);
        if (!cv::imwrite("my_bezier_curve.png", window)) {
          throw std::runtime_error("Failed to save image: my_bezier_curve.png");
        }
        cv::waitKey(0);
        break;
      }

      cv::imshow("Bezier Curve", window);
      if (cv::waitKey(20) == 27 || cv::getWindowProperty("Bezier Curve", cv::WND_PROP_VISIBLE) < 1.0) {
        break;
      }
    }
    cv::destroyAllWindows();
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << "\n";
    return 1;
  }
}
