//
// Created by Göksu Güvendiren on 2019-05-14.
//

#pragma once

#include <cstdint>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>

#include "area_light.hpp"
#include "bvh.hpp"
#include "global.hpp"
#include "light.hpp"
#include "object.hpp"
#include "ray.hpp"
#include "vector.hpp"

class Scene {
public:
  Scene(int width, int height) : m_width(width), m_height(height) {}
  // Objects are non-owning references; keep them alive while the scene is in use.
  void Add(Object* object) { m_objects.push_back(object); }
  void Add(std::unique_ptr<Light> light) { m_lights.push_back(std::move(light)); }
  int GetWidth() const { return m_width; }
  int GetHeight() const { return m_height; }
  float GetFov() const { return m_fov; }
  float GetRussianRoulette() const { return m_russian_roulette; }
  const std::vector<Object*>& GetObjects() const { return m_objects; }
  const std::vector<std::unique_ptr<Light>>& GetLights() const { return m_lights; }
  Intersection Intersect(const Ray& ray) const;
  void BuildBVH();
  void SampleLight(Intersection& pos, float& pdf) const;
  Vector3f CastRay(const Ray& ray, int depth) const;
  bool Trace(const Ray& ray, const std::vector<Object*>& objects, float& t_near, uint32_t& index, Object** hit_object);

  std::tuple<Vector3f, Vector3f> HandleAreaLight(const AreaLight& light, const Vector3f& hit_point,
                                                 const Vector3f& normal, const Vector3f& shadow_point_orig,
                                                 const std::vector<Object*>& objects, uint32_t& index,
                                                 const Vector3f& dir, float specular_exponent);

private:
  int m_width = 1280;
  int m_height = 960;
  float m_fov = 40.0F;
  [[maybe_unused]] Vector3f m_background_color{0.235294F, 0.67451F, 0.843137F};
  [[maybe_unused]] int m_max_depth = 1;
  float m_russian_roulette = 0.8F;
  std::vector<Object*> m_objects;
  std::vector<std::unique_ptr<Light>> m_lights;
  std::unique_ptr<BVHAccel> m_bvh;

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
  Vector3f Refract(const Vector3f& incident, const Vector3f& normal, const float& ior) const {
    float cosi = Clamp(-1, 1, DotProduct(incident, normal));
    float etai = 1;
    float etat = ior;
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
  // \param ior is the material refractive index
  //
  // \param[out] kr is the amount of light reflected
  void Fresnel(const Vector3f& incident, const Vector3f& normal, const float& ior, float& kr) const {
    float cosi = Clamp(-1, 1, DotProduct(incident, normal));
    float etai = 1;
    float etat = ior;
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
};
