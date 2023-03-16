#pragma once

#include <cstdint>

#include "global.hpp"
#include "vector.hpp"

class Object {
public:
  Object() = default;

  Object(const Object&) = default;
  Object& operator=(const Object&) = default;
  Object(Object&&) noexcept = default;
  Object& operator=(Object&&) noexcept = default;
  virtual ~Object() = default;

  virtual bool Intersect(const Vector3f&, const Vector3f&, float&, uint32_t&, Vector2f&) const = 0;

  virtual void GetSurfaceProperties(const Vector3f&, const Vector3f&, const uint32_t&, const Vector2f&, Vector3f&,
                                    Vector2f&) const = 0;

  virtual Vector3f EvalDiffuseColor(const Vector2f&) const { return diffuse_color; }

  // material properties
  MaterialType material_type = DiffuseAndGlossy;
  float ior = 1.3F;
  float kd = 0.8F;
  float ks = 0.2F;
  Vector3f diffuse_color{0.2F};
  float specular_exponent = 25.0F;
};
