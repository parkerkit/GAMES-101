#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "object.hpp"

inline bool RayTriangleIntersect([[maybe_unused]] const Vector3f& v0, [[maybe_unused]] const Vector3f& v1,
                                 [[maybe_unused]] const Vector3f& v2, [[maybe_unused]] const Vector3f& orig,
                                 [[maybe_unused]] const Vector3f& dir, [[maybe_unused]] float& tnear,
                                 [[maybe_unused]] float& u, [[maybe_unused]] float& v) {
  // TODO: Implement this function that tests whether the triangle
  // that's specified by v0, v1 and v2 intersects with the ray (whose
  // origin is *orig* and direction is *dir*)
  // Also don't forget to update tnear, u and v.
  return false;
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
