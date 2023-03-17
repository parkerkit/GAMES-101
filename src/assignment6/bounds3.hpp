//
// Created by LEI XU on 5/16/19.
//

#ifndef RAYTRACING_BOUNDS3_H
#define RAYTRACING_BOUNDS3_H
#include <array>
#include <limits>

#include "ray.hpp"
#include "vector.hpp"

class Bounds3 {
public:
  Vector3f p_min;
  Vector3f p_max;  // two points to specify the bounding box
  Bounds3() : p_min(std::numeric_limits<float>::max()), p_max(std::numeric_limits<float>::lowest()) {}
  Bounds3(Vector3f p) : p_min(p), p_max(p) {}
  Bounds3(Vector3f p1, Vector3f p2) : p_min(Vector3f::Min(p1, p2)), p_max(Vector3f::Max(p1, p2)) {}

  Vector3f Diagonal() const { return p_max - p_min; }
  int MaxExtent() const {
    Vector3f d = Diagonal();
    if (d.x > d.y && d.x > d.z) {
      return 0;
    }
    if (d.y > d.z) {
      return 1;
    }
    return 2;
  }

  double SurfaceArea() const {
    Vector3f d = Diagonal();
    return 2 * (d.x * d.y + d.x * d.z + d.y * d.z);
  }

  Vector3f Centroid() const { return 0.5F * p_min + 0.5F * p_max; }
  Bounds3 Intersect(const Bounds3& b) {
    return Bounds3(Vector3f(std::max(p_min.x, b.p_min.x), std::max(p_min.y, b.p_min.y), std::max(p_min.z, b.p_min.z)),
                   Vector3f(std::min(p_max.x, b.p_max.x), std::min(p_max.y, b.p_max.y), std::min(p_max.z, b.p_max.z)));
  }

  Vector3f Offset(const Vector3f& p) const {
    Vector3f o = p - p_min;
    if (p_max.x > p_min.x) {
      o.x /= p_max.x - p_min.x;
    }
    if (p_max.y > p_min.y) {
      o.y /= p_max.y - p_min.y;
    }
    if (p_max.z > p_min.z) {
      o.z /= p_max.z - p_min.z;
    }
    return o;
  }

  bool Overlaps(const Bounds3& b1, const Bounds3& b2) {
    bool x = (b1.p_max.x >= b2.p_min.x) && (b1.p_min.x <= b2.p_max.x);
    bool y = (b1.p_max.y >= b2.p_min.y) && (b1.p_min.y <= b2.p_max.y);
    bool z = (b1.p_max.z >= b2.p_min.z) && (b1.p_min.z <= b2.p_max.z);
    return (x && y && z);
  }

  bool Inside(const Vector3f& p, const Bounds3& b) {
    return (p.x >= b.p_min.x && p.x <= b.p_max.x && p.y >= b.p_min.y && p.y <= b.p_max.y && p.z >= b.p_min.z &&
            p.z <= b.p_max.z);
  }
  inline const Vector3f& operator[](int i) const { return (i == 0) ? p_min : p_max; }

  inline bool IntersectP([[maybe_unused]] const Ray& ray, [[maybe_unused]] const Vector3f& inv_dir,
                         [[maybe_unused]] const std::array<int, 3>& dir_is_neg) const;
};

inline bool Bounds3::IntersectP([[maybe_unused]] const Ray& ray, [[maybe_unused]] const Vector3f& inv_dir,
                                [[maybe_unused]] const std::array<int, 3>& dir_is_neg) const {
  // invDir: ray direction(x,y,z), invDir=(1.0/x,1.0/y,1.0/z), use this because Multiply is faster that Division
  // dirIsNeg: ray direction(x,y,z), dirIsNeg=[int(x>0),int(y>0),int(z>0)], use this to simplify your logic
  // TODO: Test whether the ray intersects this bounding box.
  return false;
}

inline Bounds3 Union(const Bounds3& b1, const Bounds3& b2) {
  Bounds3 ret;
  ret.p_min = Vector3f::Min(b1.p_min, b2.p_min);
  ret.p_max = Vector3f::Max(b1.p_max, b2.p_max);
  return ret;
}

inline Bounds3 Union(const Bounds3& b, const Vector3f& p) {
  Bounds3 ret;
  ret.p_min = Vector3f::Min(b.p_min, p);
  ret.p_max = Vector3f::Max(b.p_max, p);
  return ret;
}

#endif  // RAYTRACING_BOUNDS3_H
