//
// Created by goksu on 4/6/19.
//

#include "rasterizer.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <limits>
#include <stdexcept>

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

// Bresenham's line drawing algorithm
// Code taken from a stack overflow answer: https://stackoverflow.com/a/16405254
void rst::Rasterizer::DrawLine(Eigen::Vector3f begin, Eigen::Vector3f end) {
  auto x1 = begin.x();
  auto y1 = begin.y();
  auto x2 = end.x();
  auto y2 = end.y();

  Eigen::Vector3f line_color = {255, 255, 255};

  int x{};
  int y{};
  int dx{};
  int dy{};
  int dx1{};
  int dy1{};
  int px{};
  int py{};
  int xe{};
  int ye{};
  int i{};

  dx = static_cast<int>(x2 - x1);
  dy = static_cast<int>(y2 - y1);
  dx1 = std::abs(dx);
  dy1 = std::abs(dy);
  px = 2 * dy1 - dx1;
  py = 2 * dx1 - dy1;

  if (dy1 <= dx1) {
    if (dx >= 0) {
      x = static_cast<int>(x1);
      y = static_cast<int>(y1);
      xe = static_cast<int>(x2);
    } else {
      x = static_cast<int>(x2);
      y = static_cast<int>(y2);
      xe = static_cast<int>(x1);
    }
    Eigen::Vector3f point = Eigen::Vector3f(static_cast<float>(x), static_cast<float>(y), 1.0F);
    SetPixel(point, line_color);
    for (i = 0; x < xe; i++) {
      x = x + 1;
      if (px < 0) {
        px = px + 2 * dy1;
      } else {
        if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0)) {
          y = y + 1;
        } else {
          y = y - 1;
        }
        px = px + 2 * (dy1 - dx1);
      }
      //            delay(0);
      point = Eigen::Vector3f(static_cast<float>(x), static_cast<float>(y), 1.0F);
      SetPixel(point, line_color);
    }
  } else {
    if (dy >= 0) {
      x = static_cast<int>(x1);
      y = static_cast<int>(y1);
      ye = static_cast<int>(y2);
    } else {
      x = static_cast<int>(x2);
      y = static_cast<int>(y2);
      ye = static_cast<int>(y1);
    }
    Eigen::Vector3f point = Eigen::Vector3f(static_cast<float>(x), static_cast<float>(y), 1.0F);
    SetPixel(point, line_color);
    for (i = 0; y < ye; i++) {
      y = y + 1;
      if (py <= 0) {
        py = py + 2 * dx1;
      } else {
        if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0)) {
          x = x + 1;
        } else {
          x = x - 1;
        }
        py = py + 2 * (dx1 - dy1);
      }
      //            delay(0);
      point = Eigen::Vector3f(static_cast<float>(x), static_cast<float>(y), 1.0F);
      SetPixel(point, line_color);
    }
  }
}

auto ToVec4(const Eigen::Vector3f& v3, float w = 1.0F) { return Eigen::Vector4f(v3.x(), v3.y(), v3.z(), w); }

void rst::Rasterizer::Draw(rst::PosBufId pos_buffer, rst::IndBufId ind_buffer, rst::Primitive type) {
  if (type != rst::Primitive::Triangle) {
    throw std::runtime_error("Drawing primitives other than triangle is not implemented yet!");
  }
  auto& buf = m_pos_buf[pos_buffer.pos_id];
  auto& ind = m_ind_buf[ind_buffer.ind_id];

  float f1 = (100.0F - 0.1F) / 2.0F;
  float f2 = (100.0F + 0.1F) / 2.0F;

  Eigen::Matrix4f mvp = m_projection * m_view * m_model;
  for (const auto& triangle_indices : ind) {
    Triangle t;

    std::array<Eigen::Vector4f, 3> v = {mvp * ToVec4(buf[triangle_indices[0]], 1.0F),
                                        mvp * ToVec4(buf[triangle_indices[1]], 1.0F),
                                        mvp * ToVec4(buf[triangle_indices[2]], 1.0F)};

    for (auto& vec : v) {
      vec /= vec.w();
    }

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

    t.SetColor(0, 255.0, 0.0, 0.0);
    t.SetColor(1, 0.0, 255.0, 0.0);
    t.SetColor(2, 0.0, 0.0, 255.0);

    RasterizeWireframe(t);
  }
}

void rst::Rasterizer::RasterizeWireframe(const Triangle& t) {
  DrawLine(t.C(), t.A());
  DrawLine(t.C(), t.B());
  DrawLine(t.B(), t.A());
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
