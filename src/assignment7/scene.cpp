// Created by Göksu Güvendiren on 2019-05-14.
#include "scene.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>

void Scene::BuildBVH() {
  std::cout << " - Generating BVH...\n\n";
  m_bvh = std::make_unique<BVHAccel>(m_objects, 1, BVHAccel::SplitMethod::Naive);
}

Intersection Scene::Intersect(const Ray& ray) const { return m_bvh ? m_bvh->Intersect(ray) : Intersection{}; }

void Scene::SampleLight(Intersection& pos, float& pdf) const {
  pos = {};
  pdf = 0.0F;
  float emit_area_sum = 0.0F;
  for (auto* object : m_objects) {
    if (object->HasEmit()) {
      emit_area_sum += object->GetArea();
    }
  }
  if (emit_area_sum <= 0.0F) {
    return;
  }
  const float total_emit_area = emit_area_sum;
  float p = GetRandomFloat() * total_emit_area;
  emit_area_sum = 0.0F;
  for (auto* object : m_objects) {
    if (object->HasEmit()) {
      emit_area_sum += object->GetArea();
      if (p <= emit_area_sum) {
        object->Sample(pos, pdf);
        // 光源表面采样密度 = 选中该发光网格的概率 × 该网格内部的采样密度。
        // 对应作业 PDF 的 pdf_light = 1 / A，A 为所有光源的总面积。
        // 原框架只返回网格内部的采样密度，这里补上选择网格的概率；单个光源时该概率为 1。
        pdf *= object->GetArea() / total_emit_area;
        break;
      }
    }
  }
}

bool Scene::Trace(const Ray& ray, const std::vector<Object*>& objects, float& t_near, uint32_t& index,
                  Object** hit_object) {
  *hit_object = nullptr;
  for (auto* object : objects) {
    float candidate_t = k_infinity;
    uint32_t candidate_index = 0;
    if (object->Intersect(ray, candidate_t, candidate_index) && candidate_t < t_near) {
      *hit_object = object;
      t_near = candidate_t;
      index = candidate_index;
    }
  }
  return *hit_object != nullptr;
}

// Implementation of Path Tracing.
Vector3f Scene::CastRay(const Ray& ray, int depth) const {
  const Intersection hit = Intersect(ray);
  if (!hit.happened || hit.m == nullptr) {
    return {};
  }
  if (hit.m->HasEmission()) {
    // 主光线直接命中光源时返回发光值。
    // 与作业 PDF 的写法不同：PDF 在递归前排除发光物体，这里在递归入口返回零来避免重复计入。
    return depth == 0 ? hit.m->GetEmission() : Vector3f();
  }

  // Cornell Box 的坐标量级约为 500，使用足以跨过 float 舍入误差的偏移，避免自相交。
  constexpr float ray_epsilon = 1e-3F;
  const Vector3f ray_origin = hit.coords + hit.normal * ray_epsilon;
  const Vector3f view_dir = -ray.direction;
  Vector3f direct;

  Intersection light_sample;
  float light_pdf = 0.0F;
  SampleLight(light_sample, light_pdf);
  if (std::isfinite(light_pdf) && light_pdf > 0.0F) {
    const Vector3f to_light = light_sample.coords - hit.coords;
    const float distance_squared = DotProduct(to_light, to_light);
    if (std::isfinite(distance_squared) && distance_squared > ray_epsilon * ray_epsilon) {
      const Vector3f light_dir = Normalize(to_light);
      const float cos_surface = std::max(0.0F, DotProduct(hit.normal, light_dir));
      // 对照 PDF 的 cos_theta_x：light_dir 从着色点指向光源，光源侧的余弦应使用 -light_dir。
      const float cos_light = std::max(0.0F, DotProduct(light_sample.normal, -light_dir));
      if (cos_surface > 0.0F && cos_light > 0.0F) {
        // PDF 用“中间无遮挡”描述可见性；这里从偏移起点发射光线，用 t_max 限制到光源之前。
        const Vector3f shadow_vector = light_sample.coords - ray_origin;
        const float shadow_distance = shadow_vector.Norm();
        Ray shadow_ray(ray_origin, Normalize(shadow_vector));
        shadow_ray.t_max = std::max(0.0, static_cast<double>(shadow_distance) - ray_epsilon);
        if (!Intersect(shadow_ray).happened) {
          // 面积采样的几何项为 cos(theta) * cos(theta_light) / distance^2。
          direct = light_sample.emit * hit.m->Eval(view_dir, light_dir, hit.normal) *
                   (cos_surface * cos_light / distance_squared / light_pdf);
        }
      }
    }
  }

  // Russian Roulette 决定是否继续间接光照，继续时除以该概率以补偿被终止的路径。
  if (GetRandomFloat() >= m_russian_roulette) {
    return direct;
  }
  const Vector3f bounce_dir = Normalize(hit.m->Sample(view_dir, hit.normal));
  const float bounce_pdf = hit.m->Pdf(view_dir, bounce_dir, hit.normal);
  const float cos_bounce = std::max(0.0F, DotProduct(hit.normal, bounce_dir));
  if (!std::isfinite(bounce_pdf) || bounce_pdf <= 0.0F || cos_bounce <= 0.0F) {
    return direct;
  }
  const Vector3f incoming = CastRay(Ray(ray_origin, bounce_dir), depth + 1);
  const Vector3f indirect =
    incoming * hit.m->Eval(view_dir, bounce_dir, hit.normal) * (cos_bounce / bounce_pdf / m_russian_roulette);
  return direct + indirect;
}
