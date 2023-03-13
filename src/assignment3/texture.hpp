// Created by LEI XU on 4/27/19.
#pragma once

#include <Eigen/Core>
#include <algorithm>
#include <cmath>
#include <opencv2/opencv.hpp>
#include <stdexcept>
#include <string>

class Texture {
private:
  cv::Mat m_image_data;
  int m_width = 0;
  int m_height = 0;

public:
  explicit Texture(const std::string& name) : m_image_data(cv::imread(name)) {
    if (m_image_data.empty()) {
      throw std::runtime_error("Failed to load texture: " + name);
    }
    cv::cvtColor(m_image_data, m_image_data, cv::COLOR_BGR2RGB);
    m_width = m_image_data.cols;
    m_height = m_image_data.rows;
  }

  int GetWidth() const { return m_width; }
  int GetHeight() const { return m_height; }

  Eigen::Vector3f GetColor(float u, float v) const {
    if (!std::isfinite(u) || !std::isfinite(v)) {
      throw std::invalid_argument("Texture coordinates must be finite");
    }
    const int x = std::min(static_cast<int>(std::clamp(u, 0.0F, 1.0F) * static_cast<float>(m_width)), m_width - 1);
    const int y =
      std::min(static_cast<int>((1.0F - std::clamp(v, 0.0F, 1.0F)) * static_cast<float>(m_height)), m_height - 1);
    const auto color = m_image_data.at<cv::Vec3b>(y, x);
    return Eigen::Vector3f(color[0], color[1], color[2]);
  }
};
