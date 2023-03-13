//
// Created by goksu on 4/6/19.
//

#include "rasterizer.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <tuple>
#include <utility>

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

rst::ColBufId rst::Rasterizer::LoadNormals(const std::vector<Eigen::Vector3f>& normals) {
  auto id = GetNextId();
  m_nor_buf.emplace(id, normals);

  m_normal_id = id;

  return {id};
}

// Bresenham's line drawing algorithm
void rst::Rasterizer::DrawLine(Eigen::Vector3f begin, Eigen::Vector3f end) {
  const int x1 = static_cast<int>(begin.x());
  const int y1 = static_cast<int>(begin.y());
  const int x2 = static_cast<int>(end.x());
  const int y2 = static_cast<int>(end.y());

  Eigen::Vector3f line_color = {255, 255, 255};

  int x = 0;
  int y = 0;
  int dx = 0;
  int dy = 0;
  int dx1 = 0;
  int dy1 = 0;
  int px = 0;
  int py = 0;
  int xe = 0;
  int ye = 0;

  dx = x2 - x1;
  dy = y2 - y1;
  dx1 = std::abs(dx);
  dy1 = std::abs(dy);
  px = 2 * dy1 - dx1;
  py = 2 * dx1 - dy1;

  if (dy1 <= dx1) {
    if (dx >= 0) {
      x = x1;
      y = y1;
      xe = x2;
    } else {
      x = x2;
      y = y2;
      xe = x1;
    }
    SetPixel(Eigen::Vector2i(x, y), line_color);
    while (x < xe) {
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
      Eigen::Vector2i point = Eigen::Vector2i(x, y);
      SetPixel(point, line_color);
    }
  } else {
    if (dy >= 0) {
      x = x1;
      y = y1;
      ye = y2;
    } else {
      x = x2;
      y = y2;
      ye = y1;
    }
    SetPixel(Eigen::Vector2i(x, y), line_color);
    while (y < ye) {
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
      Eigen::Vector2i point = Eigen::Vector2i(x, y);
      SetPixel(point, line_color);
    }
  }
}

auto ToVec4(const Eigen::Vector3f& v3, float w = 1.0F) { return Eigen::Vector4f(v3.x(), v3.y(), v3.z(), w); }

[[maybe_unused]] static bool InsideTriangle(float x, float y, const std::array<Eigen::Vector4f, 3>& vertices) {
  std::array<Eigen::Vector3f, 3> v;
  for (int i = 0; i < 3; i++) {
    v[i] = {vertices[i].x(), vertices[i].y(), 1.0F};
  }
  const Eigen::Vector3f f0 = v[1].cross(v[0]);
  const Eigen::Vector3f f1 = v[2].cross(v[1]);
  const Eigen::Vector3f f2 = v[0].cross(v[2]);
  Eigen::Vector3f p(x, y, 1.F);
  return (p.dot(f0) * f0.dot(v[2]) > 0) && (p.dot(f1) * f1.dot(v[0]) > 0) && (p.dot(f2) * f2.dot(v[1]) > 0);
}

[[maybe_unused]] static std::tuple<float, float, float> ComputeBarycentric2D(float x, float y,
                                                                             const std::array<Eigen::Vector4f, 3>& v) {
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

void rst::Rasterizer::Draw(std::vector<Triangle*>& triangle_list) {
  float f1 = (50 - 0.1F) / 2.0F;
  float f2 = (50 + 0.1F) / 2.0F;

  Eigen::Matrix4f mvp = m_projection * m_view * m_model;
  for (const auto& t : triangle_list) {
    Triangle newtri = *t;

    std::array<Eigen::Vector4f, 3> mm{(m_view * m_model * t->v[0]), (m_view * m_model * t->v[1]),
                                      (m_view * m_model * t->v[2])};

    std::array<Eigen::Vector3f, 3> viewspace_pos;

    std::transform(mm.begin(), mm.end(), viewspace_pos.begin(), [](auto& v) { return v.template head<3>(); });

    std::array<Eigen::Vector4f, 3> v = {mvp * t->v[0], mvp * t->v[1], mvp * t->v[2]};
    // Homogeneous division
    for (auto& vec : v) {
      vec.x() /= vec.w();
      vec.y() /= vec.w();
      vec.z() /= vec.w();
    }

    Eigen::Matrix4f inv_trans = (m_view * m_model).inverse().transpose();
    std::array<Eigen::Vector4f, 3> n = {inv_trans * ToVec4(t->normal[0], 0.0F), inv_trans * ToVec4(t->normal[1], 0.0F),
                                        inv_trans * ToVec4(t->normal[2], 0.0F)};

    // Viewport transformation
    for (auto& vert : v) {
      vert.x() = 0.5F * static_cast<float>(m_width) * (vert.x() + 1.0F);
      vert.y() = 0.5F * static_cast<float>(m_height) * (vert.y() + 1.0F);
      vert.z() = vert.z() * f1 + f2;
    }

    for (int i = 0; i < 3; ++i) {
      // screen space coordinates
      newtri.SetVertex(i, v[i]);
    }

    for (int i = 0; i < 3; ++i) {
      // view space normal
      newtri.SetNormal(i, n[i].head<3>());
    }

    newtri.SetColor(0, 148, 121.0F, 92.0F);
    newtri.SetColor(1, 148, 121.0F, 92.0F);
    newtri.SetColor(2, 148, 121.0F, 92.0F);

    // Also pass view space vertice position
    RasterizeTriangle(newtri, viewspace_pos);
  }
}

[[maybe_unused]] static Eigen::Vector3f Interpolate(float alpha, float beta, float gamma, const Eigen::Vector3f& vert1,
                                                    const Eigen::Vector3f& vert2, const Eigen::Vector3f& vert3,
                                                    float weight) {
  return (alpha * vert1 + beta * vert2 + gamma * vert3) / weight;
}

[[maybe_unused]] static Eigen::Vector2f Interpolate(float alpha, float beta, float gamma, const Eigen::Vector2f& vert1,
                                                    const Eigen::Vector2f& vert2, const Eigen::Vector2f& vert3,
                                                    float weight) {
  auto u = (alpha * vert1[0] + beta * vert2[0] + gamma * vert3[0]);
  auto v = (alpha * vert1[1] + beta * vert2[1] + gamma * vert3[1]);

  u /= weight;
  v /= weight;

  return Eigen::Vector2f(u, v);
}

// Screen space rasterization
void rst::Rasterizer::RasterizeTriangle([[maybe_unused]] const Triangle& t,
                                        [[maybe_unused]] const std::array<Eigen::Vector3f, 3>& view_pos) {
  // TODO: Adapt the triangle rasterization code from Assignment 2.
  // t.v contains screen-space x/y and mapped post-divide z; w retains the clip-space w.
  // Interpolate this z directly for the depth test. Match the comparison to your projection convention.
  // Use reciprocal clip-space w for perspective-correct interpolation of the attributes below.

  // TODO: Interpolate the attributes:
  // auto interpolated_color
  // auto interpolated_normal
  // auto interpolated_texcoords
  // auto interpolated_shadingcoords

  // Use: FragmentShaderPayload payload( interpolated_color, interpolated_normal.normalized(), interpolated_texcoords,
  // m_texture ? &*m_texture : nullptr); Use: payload.view_pos = interpolated_shadingcoords; Use: Instead of passing the
  // triangle's color directly to the frame buffer, pass the color to the shaders first to get the final color; Use:
  // auto pixel_color = m_fragment_shader(payload);
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
  if (w <= 0 || h <= 0) {
    throw std::invalid_argument("Rasterizer dimensions must be positive");
  }
  const auto count = static_cast<std::size_t>(w) * static_cast<std::size_t>(h);
  m_frame_buf.resize(count, Eigen::Vector3f::Zero());
  m_depth_buf.resize(count, std::numeric_limits<float>::infinity());

  m_texture = std::nullopt;
}

int rst::Rasterizer::GetIndex(int x, int y) { return (m_height - 1 - y) * m_width + x; }

void rst::Rasterizer::SetPixel(const Eigen::Vector2i& point, const Eigen::Vector3f& color) {
  if (point.x() < 0 || point.x() >= m_width || point.y() < 0 || point.y() >= m_height) {
    return;
  }
  const auto index = static_cast<std::size_t>(GetIndex(point.x(), point.y()));
  m_frame_buf[index] = color;
}

void rst::Rasterizer::SetVertexShader(std::function<Eigen::Vector3f(VertexShaderPayload)> vert_shader) {
  m_vertex_shader = std::move(vert_shader);
}

void rst::Rasterizer::SetFragmentShader(std::function<Eigen::Vector3f(FragmentShaderPayload)> frag_shader) {
  m_fragment_shader = std::move(frag_shader);
}
