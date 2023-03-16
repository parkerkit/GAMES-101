#pragma once

#include "vector.hpp"

class Light {
public:
  Light(const Vector3f& p, const Vector3f& i) : position(p), intensity(i) {}
  Light(const Light&) = default;
  Light& operator=(const Light&) = default;
  Light(Light&&) noexcept = default;
  Light& operator=(Light&&) noexcept = default;
  virtual ~Light() = default;
  Vector3f position;
  Vector3f intensity;
};
