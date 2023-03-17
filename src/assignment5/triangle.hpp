#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "object.hpp"
#include "vector.hpp"

/// @brief 使用莫勒–特朗博雷(Möller–Trumbore)算法计算射线与三角形的交点
inline bool RayTriangleIntersect(const Vector3f& v0, const Vector3f& v1, const Vector3f& v2, const Vector3f& orig,
                                 const Vector3f& dir, float& tnear, float& u, float& v) {
  constexpr float k_epsilon = 1e-8F;

  // 计算三角形的两条边
  Vector3f edge1 = v1 - v0;
  Vector3f edge2 = v2 - v0;

  // 计算叉积
  Vector3f pvec = CrossProduct(dir, edge2);

  // 计算行列式
  float det = DotProduct(edge1, pvec);

  // 射线与三角形所在平面平行，或三角形退化
  if (std::fabs(det) < k_epsilon) {
    return false;
  }

  float inv_det = 1.0F / det;

  // 计算 u
  Vector3f tvec = orig - v0;
  float uu = DotProduct(tvec, pvec) * inv_det;

  if (uu < 0.0F || uu > 1.0F) {
    return false;
  }

  // 计算 v
  Vector3f qvec = CrossProduct(tvec, edge1);
  float vv = DotProduct(dir, qvec) * inv_det;

  if (vv < 0.0F || uu + vv > 1.0F) {
    return false;
  }

  // 计算射线参数 t
  float t = DotProduct(edge2, qvec) * inv_det;

  // 只接受射线正方向上的交点
  if (t < k_epsilon) {
    return false;
  }

  // 更新输出参数
  tnear = t;
  u = uu;
  v = vv;

  return true;
}

class MeshTriangle : public Object {
public:
  MeshTriangle(const Vector3f* verts, const uint32_t* verts_index, const uint32_t& num_tris, const Vector2f* st) {
    uint32_t max_index = 0;
    for (uint32_t i = 0; i < num_tris * 3; ++i) {
      if (verts_index[i] > max_index) {
        max_index = verts_index[i];
      }
    }
    max_index += 1;
    m_vertices.resize(max_index);
    std::copy_n(verts, max_index, m_vertices.begin());
    m_vertex_index.resize(static_cast<std::size_t>(num_tris) * 3);
    std::copy_n(verts_index, static_cast<std::size_t>(num_tris) * 3, m_vertex_index.begin());
    m_num_triangles = num_tris;
    m_st_coordinates.resize(max_index);
    std::copy_n(st, max_index, m_st_coordinates.begin());
  }

  bool Intersect(const Vector3f& orig, const Vector3f& dir, float& tnear, uint32_t& index,
                 Vector2f& uv) const override {
    bool has_intersection = false;
    for (uint32_t k = 0; k < m_num_triangles; ++k) {
      const Vector3f& v0 = m_vertices[m_vertex_index[k * 3]];
      const Vector3f& v1 = m_vertices[m_vertex_index[k * 3 + 1]];
      const Vector3f& v2 = m_vertices[m_vertex_index[k * 3 + 2]];
      float t = 0;
      float u = 0;
      float v = 0;
      if (RayTriangleIntersect(v0, v1, v2, orig, dir, t, u, v) && t < tnear) {
        tnear = t;
        uv.x = u;
        uv.y = v;
        index = k;
        has_intersection = true;
      }
    }

    return has_intersection;
  }

  void GetSurfaceProperties(const Vector3f&, const Vector3f&, const uint32_t& index, const Vector2f& uv,
                            Vector3f& normal, Vector2f& st) const override {
    const Vector3f& v0 = m_vertices[m_vertex_index[index * 3]];
    const Vector3f& v1 = m_vertices[m_vertex_index[index * 3 + 1]];
    const Vector3f& v2 = m_vertices[m_vertex_index[index * 3 + 2]];
    Vector3f e0 = Normalize(v1 - v0);
    Vector3f e1 = Normalize(v2 - v1);
    normal = Normalize(CrossProduct(e0, e1));
    const Vector2f& st0 = m_st_coordinates[m_vertex_index[index * 3]];
    const Vector2f& st1 = m_st_coordinates[m_vertex_index[index * 3 + 1]];
    const Vector2f& st2 = m_st_coordinates[m_vertex_index[index * 3 + 2]];
    st = st0 * (1 - uv.x - uv.y) + st1 * uv.x + st2 * uv.y;
  }

  Vector3f EvalDiffuseColor(const Vector2f& st) const override {
    float scale = 5;
    float pattern = (fmodf(st.x * scale, 1) > 0.5F) != (fmodf(st.y * scale, 1) > 0.5F);
    return Lerp(Vector3f(0.815F, 0.235F, 0.031F), Vector3f(0.937F, 0.937F, 0.231F), pattern);
  }

private:
  std::vector<Vector3f> m_vertices;
  uint32_t m_num_triangles = 0;
  std::vector<uint32_t> m_vertex_index;
  std::vector<Vector2f> m_st_coordinates;
};
