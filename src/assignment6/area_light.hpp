//
// Created by Göksu Güvendiren on 2019-05-14.
//

#pragma once

#include "global.hpp"
#include "light.hpp"
#include "vector.hpp"

class AreaLight : public Light {
public:
  AreaLight(const Vector3f& position, const Vector3f& intensity) : Light(position, intensity) {}

  Vector3f SamplePoint() const {
    auto random_u = GetRandomFloat();
    auto random_v = GetRandomFloat();
    return position + random_u * u + random_v * v;
  }

  float length = 100.0F;
  Vector3f normal{0, -1, 0};
  Vector3f u{1, 0, 0};
  Vector3f v{0, 0, 1};
};
