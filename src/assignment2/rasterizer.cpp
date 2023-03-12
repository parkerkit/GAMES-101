//
// Created by goksu on 4/6/19.
//

#include "rasterizer.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <limits>
#include <stdexcept>
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

[[maybe_unused]] static bool InsideTriangle([[maybe_unused]] int x, [[maybe_unused]] int y,
                                            [[maybe_unused]] const std::array<Eigen::Vector3f, 3>& vertices) {
  // TODO : Implement this function to check if the point (x, y) is inside the triangle represented by vertices[0],
  // vertices[1], vertices[2]
  throw std::logic_error("TODO: Implement InsideTriangle.");
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
      t.SetVertex(i, v[i].head<3>());
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
  [[maybe_unused]] auto v = t.ToVector4();

  // TODO : Find out the bounding box of current triangle.
  // iterate through the pixel and find if the current pixel is inside the triangle

  // If so, use the following code to get the interpolated z value.
  // auto[alpha, beta, gamma] = ComputeBarycentric2D(x, y, t.v);
  // float w_reciprocal = 1.0/(alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
  // float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
  // z_interpolated *= w_reciprocal;

  // TODO : set the current pixel (use the SetPixel function) to the color of the triangle (use GetColor function) if it
  // should be painted.
}

void rst::Rasterizer::SetModel(const Eigen::Matrix4f& m) { m_model = m; }

void rst::Rasterizer::SetView(const Eigen::Matrix4f& v) { m_view = v; }

void rst::Rasterizer::SetProjection(const Eigen::Matrix4f& p) { m_projection = p; }

void rst::Rasterizer::Clear(rst::Buffers buff) {
  if ((buff & rst::Buffers::Color) == rst::Buffers::Color) {
    std::fill(m_frame_buf.begin(), m_frame_buf.end(), Eigen::Vector3f{0, 0, 0});
  }
  if ((buff & rst::Buffers::Depth) == rst::Buffers::Depth) {
    std::fill(m_depth_buf.begin(), m_depth_buf.end(), std::numeric_limits<float>::infinity());
  }
}

rst::Rasterizer::Rasterizer(int w, int h) : m_width(w), m_height(h) {
  m_frame_buf.resize(w * h);
  m_depth_buf.resize(w * h);
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
