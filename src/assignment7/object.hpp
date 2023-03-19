//
// Created by LEI XU on 5/13/19.
//
#pragma once
#ifndef RAYTRACING_OBJECT_H
#define RAYTRACING_OBJECT_H

#include <cstdint>

#include "bounds3.hpp"
#include "global.hpp"
#include "intersection.hpp"
#include "ray.hpp"
#include "vector.hpp"

class Object {
public:
  Object() = default;
  Object(const Object&) = default;
  Object& operator=(const Object&) = default;
  Object(Object&&) noexcept = default;
  Object& operator=(Object&&) noexcept = default;
  virtual ~Object() = default;
  virtual bool Intersect(const Ray& ray) = 0;
  virtual bool Intersect(const Ray& ray, float&, uint32_t&) const = 0;
  virtual Intersection GetIntersection(Ray ray) = 0;
  virtual void GetSurfaceProperties(const Vector3f&, const Vector3f&, const uint32_t&, const Vector2f&, Vector3f&,
                                    Vector2f&) const = 0;
  virtual Vector3f EvalDiffuseColor(const Vector2f&) const = 0;
  virtual Bounds3 GetBounds() = 0;
  virtual float GetArea() = 0;
  virtual void Sample(Intersection& pos, float& pdf) = 0;
  virtual bool HasEmit() = 0;
};

#endif  // RAYTRACING_OBJECT_H
