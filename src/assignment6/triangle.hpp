#pragma once

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "OBJ_Loader.h"
#include "bvh.hpp"
#include "intersection.hpp"
#include "material.hpp"
#include "object.hpp"

inline bool RayTriangleIntersect(const Vector3f& v0, const Vector3f& v1, const Vector3f& v2, const Vector3f& orig,
                                 const Vector3f& dir, float& tnear, float& u, float& v) {
  Vector3f edge1 = v1 - v0;
  Vector3f edge2 = v2 - v0;
  Vector3f pvec = CrossProduct(dir, edge2);
  float det = DotProduct(edge1, pvec);
  if (det == 0 || det < 0) {
    return false;
  }

  Vector3f tvec = orig - v0;
  u = DotProduct(tvec, pvec);
  if (u < 0 || u > det) {
    return false;
  }

  Vector3f qvec = CrossProduct(tvec, edge1);
  v = DotProduct(dir, qvec);
  if (v < 0 || u + v > det) {
    return false;
  }

  float inv_det = 1 / det;

  tnear = DotProduct(edge2, qvec) * inv_det;
  u *= inv_det;
  v *= inv_det;

  return true;
}

class Triangle : public Object {
public:
  Vector3f v0;
  Vector3f v1;
  Vector3f v2;  // vertices A, B ,C , counter-clockwise order
  Vector3f e1;
  Vector3f e2;  // 2 edges v1-v0, v2-v0;
  Vector3f t0;
  Vector3f t1;
  Vector3f t2;  // texture coords
  Vector3f normal;
  Material* m = nullptr;

  Triangle(Vector3f a, Vector3f b, Vector3f c, Material* material = nullptr)
      : v0(a), v1(b), v2(c), e1(v1 - v0), e2(v2 - v0), normal(Normalize(CrossProduct(e1, e2))), m(material) {}

  bool Intersect(const Ray& ray) override;
  bool Intersect(const Ray& ray, float& tnear, uint32_t& index) const override;
  Intersection GetIntersection(Ray ray) override;
  void GetSurfaceProperties(const Vector3f&, const Vector3f&, const uint32_t&, const Vector2f&,
                            Vector3f& surface_normal, Vector2f&) const override {
    surface_normal = normal;
  }
  Vector3f EvalDiffuseColor(const Vector2f&) const override;
  Bounds3 GetBounds() override;
};

class MeshTriangle : public Object {
public:
  explicit MeshTriangle(const std::string& filename) {
    objl::Loader loader;
    if (!loader.LoadFile(filename) || loader.LoadedMeshes.size() != 1) {
      throw std::runtime_error("Failed to load a single-mesh OBJ: " + filename);
    }
    const auto& mesh = loader.LoadedMeshes.front();
    if (mesh.Indices.empty() || mesh.Indices.size() % 3 != 0) {
      throw std::runtime_error("OBJ mesh must contain triangle indices: " + filename);
    }
    m_material.kd = 0.6F;
    m_material.ks = 0.0F;
    m_material.specular_exponent = 0.0F;
    m_triangles.reserve(mesh.Indices.size() / 3);
    for (std::size_t i = 0; i < mesh.Indices.size(); i += 3) {
      std::array<Vector3f, 3> face_vertices;
      for (std::size_t j = 0; j < 3; ++j) {
        const auto& vertex = mesh.Vertices.at(mesh.Indices[i + j]);
        face_vertices[j] = Vector3f(vertex.Position.X, vertex.Position.Y, vertex.Position.Z) * 60.0F;
        m_bounding_box = Union(m_bounding_box, face_vertices[j]);
      }
      m_triangles.emplace_back(face_vertices[0], face_vertices[1], face_vertices[2], &m_material);
    }
    std::vector<Object*> primitives;
    primitives.reserve(m_triangles.size());
    for (auto& triangle : m_triangles) {
      primitives.push_back(&triangle);
    }
    m_bvh = std::make_unique<BVHAccel>(std::move(primitives));
  }
  MeshTriangle(const MeshTriangle&) = delete;
  MeshTriangle& operator=(const MeshTriangle&) = delete;
  MeshTriangle(MeshTriangle&&) = delete;
  MeshTriangle& operator=(MeshTriangle&&) = delete;
  ~MeshTriangle() override = default;

  bool Intersect(const Ray& ray) override { return GetIntersection(ray).happened; }
  bool Intersect(const Ray& ray, float& tnear, uint32_t& index) const override {
    bool hit = false;
    for (std::size_t k = 0; k < m_triangles.size(); ++k) {
      const auto& triangle = m_triangles[k];
      float t = 0;
      float u = 0;
      float v = 0;
      if (RayTriangleIntersect(triangle.v0, triangle.v1, triangle.v2, ray.origin, ray.direction, t, u, v) && t >= 0 &&
          t < tnear) {
        tnear = t;
        index = static_cast<uint32_t>(k);
        hit = true;
      }
    }
    return hit;
  }
  Bounds3 GetBounds() override { return m_bounding_box; }
  void GetSurfaceProperties(const Vector3f& point, const Vector3f& incident, const uint32_t& index, const Vector2f& uv,
                            Vector3f& normal, Vector2f& st) const override {
    m_triangles.at(index).GetSurfaceProperties(point, incident, index, uv, normal, st);
  }
  Vector3f EvalDiffuseColor(const Vector2f& st) const override {
    float scale = 5;
    float pattern = (std::fmod(st.x * scale, 1.0F) > 0.5F) != (std::fmod(st.y * scale, 1.0F) > 0.5F);
    return Lerp(Vector3f(0.815F, 0.235F, 0.031F), Vector3f(0.937F, 0.937F, 0.231F), pattern);
  }
  Intersection GetIntersection(Ray ray) override { return m_bvh ? m_bvh->Intersect(ray) : Intersection{}; }

private:
  Bounds3 m_bounding_box;
  Material m_material{DiffuseAndGlossy, Vector3f(0.5F), Vector3f(0)};
  std::vector<Triangle> m_triangles;
  std::unique_ptr<BVHAccel> m_bvh;
};

inline bool Triangle::Intersect(const Ray& ray) { return GetIntersection(ray).happened; }
inline bool Triangle::Intersect([[maybe_unused]] const Ray& ray, [[maybe_unused]] float& tnear,
                                [[maybe_unused]] uint32_t& index) const {
  return false;
}

inline Bounds3 Triangle::GetBounds() { return Union(Bounds3(v0, v1), v2); }

inline Intersection Triangle::GetIntersection(Ray ray) {
  Intersection inter;

  if (DotProduct(ray.direction, normal) > 0) {
    return inter;
  }
  double u = 0;
  double v = 0;
  [[maybe_unused]] double t_tmp = 0;
  Vector3f pvec = CrossProduct(ray.direction, e2);
  double det = DotProduct(e1, pvec);
  if (std::abs(det) < k_epsilon) {
    return inter;
  }

  double det_inv = 1.0F / det;
  Vector3f tvec = ray.origin - v0;
  u = DotProduct(tvec, pvec) * det_inv;
  if (u < 0 || u > 1) {
    return inter;
  }
  Vector3f qvec = CrossProduct(tvec, e1);
  v = DotProduct(ray.direction, qvec) * det_inv;
  if (v < 0 || u + v > 1) {
    return inter;
  }
  t_tmp = DotProduct(e2, qvec) * det_inv;

  // TODO: Reject invalid t_tmp and populate happened, distance, coords, normal, obj, and m.

  return inter;
}

inline Vector3f Triangle::EvalDiffuseColor(const Vector2f&) const { return Vector3f(0.5F, 0.5F, 0.5F); }
