// Created by LEI XU on 5/16/19.
#pragma once

#include <limits>
#include <ostream>

#include "vector.hpp"

struct Ray {
  Vector3f origin;
  Vector3f direction;
  Vector3f direction_inv;
  double t = 0.0;
  double t_min = 0.0;
  double t_max = std::numeric_limits<double>::max();

  Ray(const Vector3f& ori, const Vector3f& dir, double time = 0.0)
      : origin(ori), direction(dir), direction_inv(1.0F / dir.x, 1.0F / dir.y, 1.0F / dir.z), t(time) {}

  Vector3f operator()(double distance) const { return origin + direction * static_cast<float>(distance); }

  friend std::ostream& operator<<(std::ostream& os, const Ray& ray) {
    return os << "[origin:=" << ray.origin << ", direction=" << ray.direction << ", time=" << ray.t << "]\n";
  }
};
