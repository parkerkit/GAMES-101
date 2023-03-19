// Created by LEI XU on 5/16/19.
#pragma once

#include <memory>
#include <vector>

#include "bounds3.hpp"
#include "intersection.hpp"
#include "object.hpp"
#include "ray.hpp"

struct BVHBuildNode {
  Bounds3 bounds;
  std::unique_ptr<BVHBuildNode> left;
  std::unique_ptr<BVHBuildNode> right;
  Object* object = nullptr;
  float area = 0.0F;
  int split_axis = 0;
  int first_prim_offset = 0;
  int n_primitives = 0;
};

class BVHAccel {
public:
  enum class SplitMethod { Naive, Sah };
  explicit BVHAccel(std::vector<Object*> primitives, int max_prims_in_node = 1,
                    SplitMethod split_method = SplitMethod::Naive);
  Bounds3 WorldBound() const { return m_root ? m_root->bounds : Bounds3(); }
  Intersection Intersect(const Ray& ray) const;
  Intersection GetIntersection(const BVHBuildNode* node, const Ray& ray) const;
  bool IntersectP(const Ray& ray) const { return Intersect(ray).happened; }
  void Sample(Intersection& pos, float& pdf);

private:
  std::unique_ptr<BVHBuildNode> RecursiveBuild(std::vector<Object*> objects);
  void GetSample(const BVHBuildNode* node, float p, Intersection& pos, float& pdf);
  [[maybe_unused]] const int m_max_prims_in_node;
  [[maybe_unused]] const SplitMethod m_split_method;
  std::vector<Object*> m_primitives;
  std::unique_ptr<BVHBuildNode> m_root;
};
