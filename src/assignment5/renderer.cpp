#include "renderer.hpp"

#include <array>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <optional>
#include <stdexcept>

#include "scene.hpp"
#include "vector.hpp"

namespace {

float DegToRad(const float& deg) { return deg * k_pi / 180.0F; }

// Compute reflection direction
Vector3f Reflect(const Vector3f& incident, const Vector3f& normal) {
  return incident - 2 * DotProduct(incident, normal) * normal;
}

// [comment]
// Compute refraction direction using Snell's law
//
// We need to handle with care the two possible situations:
//
//    - When the ray is inside the object
//
//    - When the ray is outside.
//
// If the ray is outside, you need to make cosi positive cosi = -normal.incident
//
// If the ray is inside, you need to invert the refractive indices and negate the normal
// [/comment]
Vector3f Refract(const Vector3f& incident, const Vector3f& normal, const float& ior) {
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

// [comment]
// Compute Fresnel equation
//
// \param incident is the incident view direction
//
// \param normal is the normal at the intersection point
//
// \param ior is the material refractive index
// [/comment]
float Fresnel(const Vector3f& incident, const Vector3f& normal, const float& ior) {
  float cosi = Clamp(-1, 1, DotProduct(incident, normal));
  float etai = 1;
  float etat = ior;
  if (cosi > 0) {
    std::swap(etai, etat);
  }
  // Compute sini using Snell's law
  float sint = etai / etat * sqrtf(std::max(0.0F, 1 - cosi * cosi));
  // Total internal reflection
  if (sint >= 1) {
    return 1;
  }
  float cost = sqrtf(std::max(0.0F, 1 - sint * sint));
  cosi = fabsf(cosi);
  float rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
  float rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));
  return (rs * rs + rp * rp) / 2;
  // As a consequence of the conservation of energy, transmittance is given by:
  // kt = 1 - kr;
}

// Find the closest object hit, including the distance and mesh barycentric coordinates.
std::optional<HitPayload> Trace(const Vector3f& orig, const Vector3f& dir,
                                const std::vector<std::unique_ptr<Object>>& objects) {
  float t_near = k_infinity;
  std::optional<HitPayload> payload;
  for (const auto& object : objects) {
    float t_near_k = k_infinity;
    uint32_t index_k = 0;
    Vector2f uv_k;
    if (object->Intersect(orig, dir, t_near_k, index_k, uv_k) && t_near_k < t_near) {
      payload.emplace();
      payload->hit_obj = object.get();
      payload->t_near = t_near_k;
      payload->index = index_k;
      payload->uv = uv_k;
      t_near = t_near_k;
    }
  }

  return payload;
}

// [comment]
// Implementation of the Whitted-style light transport algorithm (E [S*] (D|G) L)
//
// This function is the function that compute the color at the intersection point
// of a ray defined by a position and a direction. Note that thus function is recursive (it calls itself).
//
// If the material of the intersected object is either reflective or reflective and refractive,
// then we compute the reflection/refraction direction and cast two new rays into the scene
// by calling the CastRay() function recursively. When the surface is transparent, we mix
// the reflection and refraction color using the result of the Fresnel equations (it computes
// the amount of reflection and refraction depending on the surface normal, incident view direction
// and surface refractive index).
//
// If the surface is diffuse/glossy we use the Phong illumation model to compute the color
// at the intersection point.
// [/comment]
Vector3f CastRay(const Vector3f& orig, const Vector3f& dir, const Scene& scene, int depth) {
  if (depth > scene.GetMaxDepth()) {
    return Vector3f(0.0F, 0.0F, 0.0F);
  }

  Vector3f hit_color = scene.GetBackgroundColor();
  if (auto payload = Trace(orig, dir, scene.GetObjects()); payload) {
    Vector3f hit_point = orig + dir * payload->t_near;
    Vector3f normal;  // normal
    Vector2f st;      // st coordinates
    payload->hit_obj->GetSurfaceProperties(hit_point, dir, payload->index, payload->uv, normal, st);
    switch (payload->hit_obj->material_type) {
      case ReflectionAndRefraction: {
        Vector3f reflection_direction = Normalize(Reflect(dir, normal));
        Vector3f refraction_direction = Normalize(Refract(dir, normal, payload->hit_obj->ior));
        Vector3f reflection_ray_orig = (DotProduct(reflection_direction, normal) < 0)
                                         ? hit_point - normal * scene.GetEpsilon()
                                         : hit_point + normal * scene.GetEpsilon();
        Vector3f refraction_ray_orig = (DotProduct(refraction_direction, normal) < 0)
                                         ? hit_point - normal * scene.GetEpsilon()
                                         : hit_point + normal * scene.GetEpsilon();
        Vector3f reflection_color = CastRay(reflection_ray_orig, reflection_direction, scene, depth + 1);
        Vector3f refraction_color = CastRay(refraction_ray_orig, refraction_direction, scene, depth + 1);
        float kr = Fresnel(dir, normal, payload->hit_obj->ior);
        hit_color = reflection_color * kr + refraction_color * (1 - kr);
        break;
      }
      case Reflection: {
        float kr = Fresnel(dir, normal, payload->hit_obj->ior);
        Vector3f reflection_direction = Reflect(dir, normal);
        Vector3f reflection_ray_orig = (DotProduct(reflection_direction, normal) < 0)
                                         ? hit_point + normal * scene.GetEpsilon()
                                         : hit_point - normal * scene.GetEpsilon();
        hit_color = CastRay(reflection_ray_orig, reflection_direction, scene, depth + 1) * kr;
        break;
      }
      default: {
        // [comment]
        // We use the Phong illumation model int the default case. The phong model
        // is composed of a diffuse and a specular reflection component.
        // [/comment]
        Vector3f light_amt = 0;
        Vector3f specular_color = 0;
        Vector3f shadow_point_orig = (DotProduct(dir, normal) < 0) ? hit_point + normal * scene.GetEpsilon()
                                                                   : hit_point - normal * scene.GetEpsilon();
        // [comment]
        // Loop over all lights in the scene and sum their contribution up
        // We also apply the lambert cosine law
        // [/comment]
        for (auto& light : scene.GetLights()) {
          Vector3f light_dir = light->position - hit_point;
          // square of the distance between hit_point and the light
          float light_distance2 = DotProduct(light_dir, light_dir);
          light_dir = Normalize(light_dir);
          float ldotn = std::max(0.0F, DotProduct(light_dir, normal));
          // is the point in shadow, and is the nearest occluding object closer to the object than the light itself?
          auto shadow_res = Trace(shadow_point_orig, light_dir, scene.GetObjects());
          bool in_shadow = shadow_res && (shadow_res->t_near * shadow_res->t_near < light_distance2);

          light_amt += in_shadow ? 0 : light->intensity * ldotn;
          Vector3f reflection_direction = Reflect(-light_dir, normal);

          specular_color +=
            powf(std::max(0.0F, -DotProduct(reflection_direction, dir)), payload->hit_obj->specular_exponent) *
            light->intensity;
        }

        hit_color = light_amt * payload->hit_obj->EvalDiffuseColor(st) * payload->hit_obj->kd +
                    specular_color * payload->hit_obj->ks;
        break;
      }
    }
  }

  return hit_color;
}

}  // namespace

// [comment]
// The main render function. This where we iterate over all pixels in the image, generate
// primary rays and cast these rays into the scene. The content of the framebuffer is
// saved to a file.
// [/comment]
void Renderer::Render(const Scene& scene) {
  std::vector<Vector3f> framebuffer(static_cast<std::size_t>(scene.GetWidth()) *
                                    static_cast<std::size_t>(scene.GetHeight()));

  [[maybe_unused]] float scale = std::tan(DegToRad(scene.GetFov() * 0.5F));
  [[maybe_unused]] float image_aspect_ratio =
    static_cast<float>(scene.GetWidth()) / static_cast<float>(scene.GetHeight());

  // Use this variable as the eye position to start your rays.
  Vector3f eye_pos(0);
  std::size_t pixel_index = 0;
  for (int j = 0; j < scene.GetHeight(); ++j) {
    for (int i = 0; i < scene.GetWidth(); ++i) {
      // generate primary ray direction
      float x = 0.0F;
      float y = 0.0F;
      // TODO: Find the x and y positions of the current pixel to get the direction
      // vector that passes through it.
      // Also, don't forget to multiply both of them with the variable *scale*, and
      // x (horizontal) variable with the *image_aspect_ratio*

      Vector3f dir = Vector3f(x, y, -1);  // Don't forget to Normalize this direction!
      framebuffer[pixel_index++] = CastRay(eye_pos, dir, scene, 0);
    }
    UpdateProgress(static_cast<float>(j) / static_cast<float>(scene.GetHeight()));
  }

  UpdateProgress(1.0F);
  std::cout << '\n';

  // P6 stores RGB bytes; no OpenCV dependency is needed for this assignment.
  std::ofstream output("binary.ppm", std::ios::binary);
  if (!output) {
    throw std::runtime_error("Failed to open binary.ppm for writing");
  }
  output << "P6\n" << scene.GetWidth() << ' ' << scene.GetHeight() << "\n255\n";
  for (const auto& pixel : framebuffer) {
    const std::array<unsigned char, 3> color{static_cast<unsigned char>(255.0F * Clamp(0, 1, pixel.x)),
                                             static_cast<unsigned char>(255.0F * Clamp(0, 1, pixel.y)),
                                             static_cast<unsigned char>(255.0F * Clamp(0, 1, pixel.z))};
    for (const unsigned char channel : color) {
      output.put(static_cast<char>(channel));
    }
  }
  output.close();
  if (!output) {
    throw std::runtime_error("Failed to write binary.ppm");
  }
}
