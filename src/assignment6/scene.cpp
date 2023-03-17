//
// Created by Göksu Güvendiren on 2019-05-14.
//

#include "scene.hpp"

#include <cstddef>

void Scene::BuildBVH() {
  std::cout << "Generating BVH...\n";
  m_bvh = std::make_unique<BVHAccel>(m_objects, 1, BVHAccel::SplitMethod::Naive);
}

Intersection Scene::Intersect(const Ray& ray) const { return m_bvh ? m_bvh->Intersect(ray) : Intersection{}; }

bool Scene::Trace(const Ray& ray, const std::vector<Object*>& objects, float& t_near, uint32_t& index,
                  Object** hit_object) {
  *hit_object = nullptr;
  for (auto* object : objects) {
    float t_near_k = k_infinity;
    uint32_t index_k = 0;
    if (object->Intersect(ray, t_near_k, index_k) && t_near_k < t_near) {
      *hit_object = object;
      t_near = t_near_k;
      index = index_k;
    }
  }

  return (*hit_object != nullptr);
}

// Implementation of the Whitted-syle light transport algorithm (E [S*] (D|G) L)
//
// This function is the function that compute the color at the intersection point
// of a ray defined by a position and a direction. Note that thus function is recursive (it calls itself).
//
// If the material of the intersected object is either reflective or reflective and refractive,
// then we compute the reflection/refracton direction and cast two new rays into the scene
// by calling the castRay() function recursively. When the surface is transparent, we mix
// the reflection and refraction color using the result of the fresnel equations (it computes
// the amount of reflection and refractin depending on the surface normal, incident view direction
// and surface refractive index).
//
// If the surface is duffuse/glossy we use the Phong illumation model to compute the color
// at the intersection point.
Vector3f Scene::CastRay(const Ray& ray, int depth) const {
  if (depth > m_max_depth) {
    return Vector3f(0.0F, 0.0F, 0.0F);
  }
  Intersection intersection = Scene::Intersect(ray);
  Material* m = intersection.m;
  Object* hit_object = intersection.obj;
  Vector3f hit_color = m_background_color;
  Vector2f uv;
  uint32_t index = 0;
  if (intersection.happened) {
    Vector3f hit_point = intersection.coords;
    Vector3f normal = intersection.normal;  // normal
    Vector2f st;                            // st coordinates
    hit_object->GetSurfaceProperties(hit_point, ray.direction, index, uv, normal, st);
    switch (m->GetType()) {
      case ReflectionAndRefraction: {
        Vector3f reflection_direction = Normalize(Reflect(ray.direction, normal));
        Vector3f refraction_direction = Normalize(Refract(ray.direction, normal, m->ior));
        Vector3f reflection_ray_orig = (DotProduct(reflection_direction, normal) < 0) ? hit_point - normal * k_epsilon
                                                                                      : hit_point + normal * k_epsilon;
        Vector3f refraction_ray_orig = (DotProduct(refraction_direction, normal) < 0) ? hit_point - normal * k_epsilon
                                                                                      : hit_point + normal * k_epsilon;
        Vector3f reflection_color = CastRay(Ray(reflection_ray_orig, reflection_direction), depth + 1);
        Vector3f refraction_color = CastRay(Ray(refraction_ray_orig, refraction_direction), depth + 1);
        float kr = 0;
        Fresnel(ray.direction, normal, m->ior, kr);
        hit_color = reflection_color * kr + refraction_color * (1 - kr);
        break;
      }
      case Reflection: {
        float kr = 0;
        Fresnel(ray.direction, normal, m->ior, kr);
        Vector3f reflection_direction = Reflect(ray.direction, normal);
        Vector3f reflection_ray_orig = (DotProduct(reflection_direction, normal) < 0) ? hit_point + normal * k_epsilon
                                                                                      : hit_point - normal * k_epsilon;
        hit_color = CastRay(Ray(reflection_ray_orig, reflection_direction), depth + 1) * kr;
        break;
      }
      default: {
        // [comment]
        // We use the Phong illumation model int the default case. The phong model
        // is composed of a diffuse and a specular reflection component.
        // [/comment]
        Vector3f light_amt = 0;
        Vector3f specular_color = 0;
        Vector3f shadow_point_orig =
          (DotProduct(ray.direction, normal) < 0) ? hit_point + normal * k_epsilon : hit_point - normal * k_epsilon;
        // [comment]
        // Loop over all lights in the scene and sum their contribution up
        // We also apply the lambert cosine law
        // [/comment]
        for (const auto& light : GetLights()) {
          auto area_ptr = dynamic_cast<AreaLight*>(light.get());
          if (area_ptr) {
            // Do nothing for this assignment
          } else {
            Vector3f light_dir = light->position - hit_point;
            light_dir = Normalize(light_dir);
            float ldotn = std::max(0.F, DotProduct(light_dir, normal));
            // is the point in shadow, and is the nearest occluding object closer to the object than the light itself?
            bool in_shadow = m_bvh->Intersect(Ray(shadow_point_orig, light_dir)).happened;
            light_amt += (in_shadow ? 0.0F : 1.0F) * light->intensity * ldotn;
            Vector3f reflection_direction = Reflect(-light_dir, normal);
            specular_color +=
              powf(std::max(0.F, -DotProduct(reflection_direction, ray.direction)), m->specular_exponent) *
              light->intensity;
          }
        }
        hit_color = light_amt * (hit_object->EvalDiffuseColor(st) * m->kd + specular_color * m->ks);
        break;
      }
    }
  }

  return hit_color;
}
