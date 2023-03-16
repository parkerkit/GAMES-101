#pragma once

#include "object.hpp"
#include "vector.hpp"

class Sphere : public Object {
public:
  Sphere(const Vector3f& c, const float& r) : center(c), radius(r), radius2(r * r) {}

  bool Intersect(const Vector3f& orig, const Vector3f& dir, float& tnear, uint32_t&, Vector2f&) const override {
    // analytic solution
    Vector3f offset = orig - center;
    float a = DotProduct(dir, dir);
    float b = 2 * DotProduct(dir, offset);
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

  void GetSurfaceProperties(const Vector3f& point, const Vector3f&, const uint32_t&, const Vector2f&, Vector3f& normal,
                            Vector2f&) const override {
    normal = Normalize(point - center);
  }

  Vector3f center;
  float radius;
  float radius2;
};
