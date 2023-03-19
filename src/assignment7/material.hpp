// Created by LEI XU on 5/16/19.
#pragma once

#include <cmath>

#include "global.hpp"
#include "vector.hpp"

enum MaterialType { Diffuse };

class Material {
public:
  MaterialType m_type = Diffuse;
  Vector3f m_emission;
  float ior = 1.3F;
  Vector3f kd;
  Vector3f ks;
  float specular_exponent = 25.0F;

  Material(MaterialType type = Diffuse, Vector3f emission = Vector3f(0)) : m_type(type), m_emission(emission) {}
  MaterialType GetType() const { return m_type; }
  Vector3f GetEmission() const { return m_emission; }
  bool HasEmission() const { return m_emission.Norm() > k_epsilon; }
  Vector3f GetColorAt([[maybe_unused]] double u, [[maybe_unused]] double v) const { return Vector3f(); }

  // Uniform sampling on the hemisphere around the surface normal.
  Vector3f Sample([[maybe_unused]] const Vector3f& wi, const Vector3f& normal) const {
    switch (m_type) {
      case Diffuse: {
        float x1 = GetRandomFloat();
        float x2 = GetRandomFloat();
        float z = std::abs(1.0F - 2.0F * x1);
        float r = std::sqrt(1.0F - z * z);
        float phi = 2.0F * k_pi * x2;
        Vector3f local_ray(r * std::cos(phi), r * std::sin(phi), z);
        return ToWorld(local_ray, normal);
      }
    }
    return {};
  }

  float Pdf([[maybe_unused]] const Vector3f& wi, const Vector3f& wo, const Vector3f& normal) const {
    switch (m_type) {
      case Diffuse:
        return DotProduct(wo, normal) > 0.0F ? 0.5F / k_pi : 0.0F;
    }
    return 0.0F;
  }

  Vector3f Eval([[maybe_unused]] const Vector3f& wi, const Vector3f& wo, const Vector3f& normal) const {
    switch (m_type) {
      case Diffuse:
        return DotProduct(normal, wo) > 0.0F ? kd / k_pi : Vector3f(0.0F);
    }
    return {};
  }

private:
  // Compute reflection direction
  Vector3f Reflect(const Vector3f& incident, const Vector3f& normal) const {
    return incident - 2 * DotProduct(incident, normal) * normal;
  }

  // Compute refraction direction using Snell's law
  //
  // We need to handle with care the two possible situations:
  //
  //    - When the ray is inside the object
  //
  //    - When the ray is outside.
  //
  // If the ray is outside, you need to make cosi positive cosi = -N.I
  //
  // If the ray is inside, you need to invert the refractive indices and negate the normal N
  Vector3f Refract(const Vector3f& incident, const Vector3f& normal, const float& refractive_index) const {
    float cosi = Clamp(-1, 1, DotProduct(incident, normal));
    float etai = 1;
    float etat = refractive_index;
    Vector3f n = normal;
    if (cosi < 0) {
      cosi = -cosi;
    } else {
      std::swap(etai, etat);
      n = -normal;
    }
    float eta = etai / etat;
    float k = 1 - eta * eta * (1 - cosi * cosi);
    return k < 0 ? 0 : eta * incident + (eta * cosi - sqrtf(k)) * n;
  }

  // Compute Fresnel equation
  //
  // \param I is the incident view direction
  //
  // \param N is the normal at the intersection point
  //
  // \param refractive_index is the material refractive index
  //
  // \param[out] kr is the amount of light reflected
  void Fresnel(const Vector3f& incident, const Vector3f& normal, const float& refractive_index, float& kr) const {
    float cosi = Clamp(-1, 1, DotProduct(incident, normal));
    float etai = 1;
    float etat = refractive_index;
    if (cosi > 0) {
      std::swap(etai, etat);
    }
    // Compute sini using Snell's law
    float sint = etai / etat * sqrtf(std::max(0.F, 1 - cosi * cosi));
    // Total internal reflection
    if (sint >= 1) {
      kr = 1;
    } else {
      float cost = sqrtf(std::max(0.F, 1 - sint * sint));
      cosi = fabsf(cosi);
      float rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
      float rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));
      kr = (rs * rs + rp * rp) / 2;
    }
    // As a consequence of the conservation of energy, transmittance is given by:
    // kt = 1 - kr;
  }

  Vector3f ToWorld(const Vector3f& local, const Vector3f& normal) const {
    Vector3f tangent;
    if (std::abs(normal.x) > std::abs(normal.y)) {
      float inv_len = 1.0F / std::sqrt(normal.x * normal.x + normal.z * normal.z);
      tangent = Vector3f(normal.z * inv_len, 0.0F, -normal.x * inv_len);
    } else {
      float inv_len = 1.0F / std::sqrt(normal.y * normal.y + normal.z * normal.z);
      tangent = Vector3f(0.0F, normal.z * inv_len, -normal.y * inv_len);
    }
    Vector3f bitangent = CrossProduct(tangent, normal);
    return local.x * bitangent + local.y * tangent + local.z * normal;
  }
};
