// Created by Göksu Güvendiren on 2019-05-14.
#include "scene.hpp"

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
  float p = GetRandomFloat() * emit_area_sum;
  emit_area_sum = 0.0F;
  for (auto* object : m_objects) {
    if (object->HasEmit()) {
      emit_area_sum += object->GetArea();
      if (p <= emit_area_sum) {
        object->Sample(pos, pdf);
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
Vector3f Scene::CastRay([[maybe_unused]] const Ray& ray, [[maybe_unused]] int depth) const {
  // TODO: 实现路径追踪算法，包括直接光照、间接光照和 Russian Roulette。
  return {};
}
