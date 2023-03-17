// Created by LEI XU on 5/16/19.
#pragma once

#include "vector.hpp"

enum MaterialType { DiffuseAndGlossy, ReflectionAndRefraction, Reflection };

class Material {
public:
  MaterialType m_type = DiffuseAndGlossy;
  Vector3f m_color{1, 1, 1};
  Vector3f m_emission;
  float ior = 1.3F;
  float kd = 0.8F;
  float ks = 0.2F;
  float specular_exponent = 25.0F;

  Material(MaterialType type = DiffuseAndGlossy, Vector3f color = Vector3f(1, 1, 1), Vector3f emission = Vector3f(0))
      : m_type(type), m_color(color), m_emission(emission) {}
  MaterialType GetType() const { return m_type; }
  Vector3f GetColor() const { return m_color; }
  Vector3f GetEmission() const { return m_emission; }
  Vector3f GetColorAt([[maybe_unused]] double u, [[maybe_unused]] double v) const { return Vector3f(); }
};
