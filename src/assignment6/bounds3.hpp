//
// Created by LEI XU on 5/16/19.
//

#ifndef RAYTRACING_BOUNDS3_H
#define RAYTRACING_BOUNDS3_H
#include <algorithm>
#include <array>
#include <cmath>
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

  inline bool IntersectP(const Ray& ray, const Vector3f& inv_dir, const std::array<int, 3>& dir_is_neg) const;
};

inline bool Bounds3::IntersectP(const Ray& ray, const Vector3f& inv_dir, const std::array<int, 3>& dir_is_neg) const {
  // dir_is_neg 沿用课程框架约定：分量为正时记为 1，为负或零时记为 0。
  // slab 是某一坐标轴上由包围盒两侧平面夹出的区域；三个 slab 的有效 t 区间必须重叠。
  // t_enter 取各轴进入时刻的最大值，t_exit 取离开时刻的最小值，并限制在射线有效范围内。
  double t_enter = std::max(0.0, ray.t_min);
  double t_exit = ray.t_max;
  if (std::isnan(t_enter) || std::isnan(t_exit) || t_enter > t_exit) {
    return false;
  }
  for (int axis = 0; axis < 3; ++axis) {
    const float origin = ray.origin[axis];
    const float direction = ray.direction[axis];
    if (!std::isfinite(origin) || !std::isfinite(direction) || p_min[axis] > p_max[axis]) {
      return false;
    }
    // 平行于 slab 时，该轴坐标恒定：原点在区间外则永不相交，在区间内则不限制 t。
    // 单独处理也避免边界上的 0 * infinity 产生 NaN。
    if (direction == 0.0F) {
      if (origin < p_min[axis] || origin > p_max[axis]) {
        return false;
      }
      continue;
    }
    // 优先复用方向倒数；极小的非零分量若使 float 倒数溢出，则用 double 重新计算。
    const double inverse =
      std::isfinite(inv_dir[axis]) ? static_cast<double>(inv_dir[axis]) : 1.0 / static_cast<double>(direction);
    // 正向先进入 p_min 平面，负向先进入 p_max 平面，确保 near_t <= far_t。
    const int near_side = dir_is_neg[axis] != 0 ? 0 : 1;
    const double near_t = (static_cast<double>((*this)[near_side][axis]) - origin) * inverse;
    const double far_t = (static_cast<double>((*this)[1 - near_side][axis]) - origin) * inverse;
    t_enter = std::max(t_enter, near_t);
    t_exit = std::min(t_exit, far_t);
    // 区间不重叠即可提前退出；使用 > 而非 >=，保留相切及零厚度包围盒的交点。
    if (t_enter > t_exit) {
      return false;
    }
  }
  return true;
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
