//
// Created by goksu on 4/6/19.
//

#include "rasterizer.hpp"

#include <Eigen/Core>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <limits>
#include <tuple>
#include <vector>

rst::PosBufId rst::Rasterizer::LoadPositions(const std::vector<Eigen::Vector3f>& positions) {
  auto id = GetNextId();
  m_pos_buf.emplace(id, positions);

  return {id};
}

rst::IndBufId rst::Rasterizer::LoadIndices(const std::vector<Eigen::Vector3i>& indices) {
  auto id = GetNextId();
  m_ind_buf.emplace(id, indices);

  return {id};
}

rst::ColBufId rst::Rasterizer::LoadColors(const std::vector<Eigen::Vector3f>& cols) {
  auto id = GetNextId();
  m_col_buf.emplace(id, cols);

  return {id};
}

auto ToVec4(const Eigen::Vector3f& v3, float w = 1.0F) { return Eigen::Vector4f(v3.x(), v3.y(), v3.z(), w); }

static bool InsideTriangle(float x, float y, const std::array<Eigen::Vector3f, 3>& vertices) {
  // 使用 叉积法 判断点是否在三角形内
  // 三个结果都非负，或者都非正：点在三角形内部或边界上。
  // 结果有正有负：点在三角形外部。
  Eigen::Vector2f p(x, y);

  Eigen::Vector2f a = vertices[0].head<2>();
  Eigen::Vector2f b = vertices[1].head<2>();
  Eigen::Vector2f c = vertices[2].head<2>();

  auto cross_2d = [](const Eigen::Vector2f& u, const Eigen::Vector2f& v) { return u.x() * v.y() - u.y() * v.x(); };

  float c1 = cross_2d(b - a, p - a);
  float c2 = cross_2d(c - b, p - b);
  float c3 = cross_2d(a - c, p - c);

  constexpr float eps = 1e-6F;

  bool has_negative = c1 < -eps || c2 < -eps || c3 < -eps;
  bool has_positive = c1 > eps || c2 > eps || c3 > eps;

  return !(has_negative && has_positive);
}

[[maybe_unused]] static std::tuple<float, float, float> ComputeBarycentric2D(float x, float y,
                                                                             const std::array<Eigen::Vector3f, 3>& v) {
  float c1 =
    (x * (v[1].y() - v[2].y()) + (v[2].x() - v[1].x()) * y + v[1].x() * v[2].y() - v[2].x() * v[1].y()) /
    (v[0].x() * (v[1].y() - v[2].y()) + (v[2].x() - v[1].x()) * v[0].y() + v[1].x() * v[2].y() - v[2].x() * v[1].y());
  float c2 =
    (x * (v[2].y() - v[0].y()) + (v[0].x() - v[2].x()) * y + v[2].x() * v[0].y() - v[0].x() * v[2].y()) /
    (v[1].x() * (v[2].y() - v[0].y()) + (v[0].x() - v[2].x()) * v[1].y() + v[2].x() * v[0].y() - v[0].x() * v[2].y());
  float c3 =
    (x * (v[0].y() - v[1].y()) + (v[1].x() - v[0].x()) * y + v[0].x() * v[1].y() - v[1].x() * v[0].y()) /
    (v[2].x() * (v[0].y() - v[1].y()) + (v[1].x() - v[0].x()) * v[2].y() + v[0].x() * v[1].y() - v[1].x() * v[0].y());
  return {c1, c2, c3};
}

void rst::Rasterizer::Draw(PosBufId pos_buffer, IndBufId ind_buffer, ColBufId col_buffer,
                           [[maybe_unused]] Primitive type) {
  auto& buf = m_pos_buf[pos_buffer.pos_id];
  auto& ind = m_ind_buf[ind_buffer.ind_id];
  auto& col = m_col_buf[col_buffer.col_id];

  float f1 = (50.0F - 0.1F) / 2.0F;
  float f2 = (50.0F + 0.1F) / 2.0F;

  Eigen::Matrix4f mvp = m_projection * m_view * m_model;
  for (const auto& triangle_indices : ind) {
    Triangle t;
    std::array<Eigen::Vector4f, 3> v = {mvp * ToVec4(buf[triangle_indices[0]], 1.0F),
                                        mvp * ToVec4(buf[triangle_indices[1]], 1.0F),
                                        mvp * ToVec4(buf[triangle_indices[2]], 1.0F)};
    // Homogeneous division
    for (auto& vec : v) {
      vec /= vec.w();
    }
    // Viewport transformation
    for (auto& vert : v) {
      vert.x() = 0.5F * static_cast<float>(m_width) * (vert.x() + 1.0F);
      vert.y() = 0.5F * static_cast<float>(m_height) * (vert.y() + 1.0F);
      vert.z() = vert.z() * f1 + f2;
    }

    for (int i = 0; i < 3; ++i) {
      t.SetVertex(i, v[i].head<3>());
    }

    auto col_x = col[triangle_indices[0]];
    auto col_y = col[triangle_indices[1]];
    auto col_z = col[triangle_indices[2]];

    t.SetColor(0, col_x[0], col_x[1], col_x[2]);
    t.SetColor(1, col_y[0], col_y[1], col_y[2]);
    t.SetColor(2, col_z[0], col_z[1], col_z[2]);

    RasterizeTriangle(t);
  }
}

// Screen space rasterization
void rst::Rasterizer::RasterizeTriangle(const Triangle& t) {
  // xy 是屏幕坐标，z 是投影后的深度值，左下角是原点 (0, 0)。
  if (m_width <= 0 || m_height <= 0) {
    return;
  }

  Eigen::Vector2f bbox_min = t.v[0].head<2>();
  Eigen::Vector2f bbox_max = bbox_min;
  for (const auto& vert : t.v) {
    if (!vert.allFinite()) {
      return;
    }
    bbox_min = bbox_min.cwiseMin(vert.head<2>());
    bbox_max = bbox_max.cwiseMax(vert.head<2>());
  }

  // 跳过退化三角形
  const Eigen::Vector2f ab = t.v[1].head<2>() - t.v[0].head<2>();
  const Eigen::Vector2f ac = t.v[2].head<2>() - t.v[0].head<2>();
  const float signed_area = ab.x() * ac.y() - ab.y() * ac.x();
  constexpr float area_epsilon = 1e-6F;
  if (!std::isfinite(signed_area) || std::abs(signed_area) <= area_epsilon) {
    return;
  }

  // 先在浮点范围内限制包围盒，避免画面外坐标转换为整数时溢出。
  const double min_x = std::max(0.0, std::floor(static_cast<double>(bbox_min.x())));
  const double min_y = std::max(0.0, std::floor(static_cast<double>(bbox_min.y())));
  const double max_x = std::min(static_cast<double>(m_width - 1), std::ceil(static_cast<double>(bbox_max.x())));
  const double max_y = std::min(static_cast<double>(m_height - 1), std::ceil(static_cast<double>(bbox_max.y())));
  if (min_x > max_x || min_y > max_y) {
    return;
  }

  for (int x = static_cast<int>(min_x); x <= static_cast<int>(max_x); ++x) {
    for (int y = static_cast<int>(min_y); y <= static_cast<int>(max_y); ++y) {
      const float sample_x = static_cast<float>(x) + 0.5F;
      const float sample_y = static_cast<float>(y) + 0.5F;
      if (!InsideTriangle(sample_x, sample_y, t.v)) {
        continue;
      }

      const auto [alpha, beta, gamma] = ComputeBarycentric2D(sample_x, sample_y, t.v);
      // 投影并除以 w 后的深度，在屏幕空间直接做线性插值。
      const float depth = alpha * t.v[0].z() + beta * t.v[1].z() + gamma * t.v[2].z();
      const auto index = static_cast<std::size_t>(GetIndex(x, y));
      // 保留当前投影约定：近处深度较大，深度缓冲以负无穷初始化。
      if (!std::isfinite(depth) || depth <= m_depth_buf[index]) {
        continue;
      }
      m_depth_buf[index] = depth;
      SetPixel(Eigen::Vector3f{sample_x, sample_y, depth}, t.GetColor());
    }
  }
}

void rst::Rasterizer::SetModel(const Eigen::Matrix4f& m) { m_model = m; }

void rst::Rasterizer::SetView(const Eigen::Matrix4f& v) { m_view = v; }

void rst::Rasterizer::SetProjection(const Eigen::Matrix4f& p) { m_projection = p; }

void rst::Rasterizer::Clear(rst::Buffers buff) {
  if ((buff & rst::Buffers::Color) == rst::Buffers::Color) {
    std::fill(m_frame_buf.begin(), m_frame_buf.end(), Eigen::Vector3f{0, 0, 0});
  }
  if ((buff & rst::Buffers::Depth) == rst::Buffers::Depth) {
    std::fill(m_depth_buf.begin(), m_depth_buf.end(), -std::numeric_limits<float>::infinity());
  }
}

rst::Rasterizer::Rasterizer(int w, int h) : m_width(w), m_height(h) {
  m_frame_buf.resize(w * h);
  m_depth_buf.resize(w * h, -std::numeric_limits<float>::infinity());
}

int rst::Rasterizer::GetIndex(int x, int y) { return (m_height - 1 - y) * m_width + x; }

void rst::Rasterizer::SetPixel(const Eigen::Vector3f& point, const Eigen::Vector3f& color) {
  if (point.x() < 0.0F || point.x() >= static_cast<float>(m_width) || point.y() < 0.0F ||
      point.y() >= static_cast<float>(m_height)) {
    return;
  }
  const int x = static_cast<int>(point.x());
  const int y = static_cast<int>(point.y());
  const auto index = static_cast<std::size_t>(GetIndex(x, y));
  m_frame_buf[index] = color;
}
