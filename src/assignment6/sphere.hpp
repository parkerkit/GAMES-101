//
// Created by LEI XU on 5/13/19.
//

#ifndef RAYTRACING_SPHERE_H
#define RAYTRACING_SPHERE_H

#include <memory>

#include "bounds3.hpp"
#include "material.hpp"
#include "object.hpp"
#include "vector.hpp"

class Sphere : public Object {
public:
  Vector3f center;
  float radius;
  float radius2;
  std::unique_ptr<Material> m;
  Sphere(const Vector3f& c, const float& r) : center(c), radius(r), radius2(r * r), m(std::make_unique<Material>()) {}
  bool Intersect(const Ray& ray) override {
    // analytic solution
    Vector3f offset = ray.origin - center;
    float a = DotProduct(ray.direction, ray.direction);
    float b = 2 * DotProduct(ray.direction, offset);
    float c = DotProduct(offset, offset) - radius2;
    float t0 = 0;
    float t1 = 0;
    if (!SolveQuadratic(a, b, c, t0, t1)) {
      return false;
    }
    if (t0 < 0) {
      t0 = t1;
    }
    if (t0 < 0) {
      return false;
    }
    return true;
  }
  bool Intersect(const Ray& ray, float& tnear, [[maybe_unused]] uint32_t& index) const override {
    // analytic solution
    Vector3f offset = ray.origin - center;
    float a = DotProduct(ray.direction, ray.direction);
    float b = 2 * DotProduct(ray.direction, offset);
    float c = DotProduct(offset, offset) - radius2;
    float t0 = 0;
    float t1 = 0;
    if (!SolveQuadratic(a, b, c, t0, t1)) {
      return false;
    }
    if (t0 < 0) {
      t0 = t1;
    }
    if (t0 < 0) {
      return false;
    }
    tnear = t0;

    return true;
  }
  Intersection GetIntersection(Ray ray) override {
    Intersection result;
    result.happened = false;
    Vector3f offset = ray.origin - center;
    float a = DotProduct(ray.direction, ray.direction);
    float b = 2 * DotProduct(ray.direction, offset);
    float c = DotProduct(offset, offset) - radius2;
    float t0 = 0;
    float t1 = 0;
    if (!SolveQuadratic(a, b, c, t0, t1)) {
      return result;
    }
    if (t0 < 0) {
      t0 = t1;
    }
    if (t0 < 0) {
      return result;
    }
    result.happened = true;

    result.coords = Vector3f(ray.origin + ray.direction * t0);
    result.normal = Normalize(Vector3f(result.coords - center));
    result.m = m.get();
    result.obj = this;
    result.distance = t0;
    return result;
  }
  void GetSurfaceProperties(const Vector3f& point, const Vector3f&, const uint32_t&, const Vector2f&, Vector3f& normal,
                            Vector2f&) const override {
    normal = Normalize(point - center);
  }

  Vector3f EvalDiffuseColor(const Vector2f&) const override { return m->GetColor(); }
  Bounds3 GetBounds() override {
    return Bounds3(Vector3f(center.x - radius, center.y - radius, center.z - radius),
                   Vector3f(center.x + radius, center.y + radius, center.z + radius));
  }
};

#endif  // RAYTRACING_SPHERE_H
