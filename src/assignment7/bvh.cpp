#include "bvh.hpp"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <utility>

BVHAccel::BVHAccel(std::vector<Object*> primitives, int max_prims_in_node, SplitMethod split_method)
    : m_max_prims_in_node(std::min(255, max_prims_in_node)),
      m_split_method(split_method),
      m_primitives(std::move(primitives)) {
  const auto start = std::chrono::steady_clock::now();
  if (m_primitives.empty()) {
    return;
  }
  m_root = RecursiveBuild(m_primitives);
  const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);
  std::cout << "BVH generation complete: " << elapsed.count() << " ms\n";
}

std::unique_ptr<BVHBuildNode> BVHAccel::RecursiveBuild(std::vector<Object*> objects) {
  auto node = std::make_unique<BVHBuildNode>();

  if (objects.size() == 1) {
    // Create leaf _BVHBuildNode_
    node->bounds = objects[0]->GetBounds();
    node->object = objects[0];
    node->area = objects[0]->GetArea();
    node->left = nullptr;
    node->right = nullptr;
    return node;
  }
  if (objects.size() == 2) {
    node->left = RecursiveBuild(std::vector{objects[0]});
    node->right = RecursiveBuild(std::vector{objects[1]});

    node->bounds = Union(node->left->bounds, node->right->bounds);
    node->area = node->left->area + node->right->area;
    return node;
  }
  {
    Bounds3 centroid_bounds;
    for (auto* object : objects) {
      centroid_bounds = Union(centroid_bounds, object->GetBounds().Centroid());
    }
    int dim = centroid_bounds.MaxExtent();
    std::sort(objects.begin(), objects.end(),
              [dim](Object* a, Object* b) { return a->GetBounds().Centroid()[dim] < b->GetBounds().Centroid()[dim]; });
    const std::size_t split_index = objects.size() / 2;
    node->split_axis = dim;

    auto beginning = objects.begin();
    auto middling = objects.begin() + static_cast<std::ptrdiff_t>(split_index);
    auto ending = objects.end();

    auto leftshapes = std::vector<Object*>(beginning, middling);
    auto rightshapes = std::vector<Object*>(middling, ending);

    assert(objects.size() == (leftshapes.size() + rightshapes.size()));

    node->left = RecursiveBuild(leftshapes);
    node->right = RecursiveBuild(rightshapes);

    node->bounds = Union(node->left->bounds, node->right->bounds);
    node->area = node->left->area + node->right->area;
  }

  return node;
}

Intersection BVHAccel::Intersect(const Ray& ray) const {
  if (!m_root) {
    return {};
  }
  return GetIntersection(m_root.get(), ray);
}

Intersection BVHAccel::GetIntersection([[maybe_unused]] const BVHBuildNode* node,
                                       [[maybe_unused]] const Ray& ray) const {
  // TODO: 遍历 BVH 并返回最近交点，可从作业 6 迁入实现。
  return {};
}

void BVHAccel::GetSample(const BVHBuildNode* node, float p, Intersection& pos, float& pdf) {
  if (node->left == nullptr || node->right == nullptr) {
    node->object->Sample(pos, pdf);
    pdf *= node->area;
    return;
  }
  if (p < node->left->area) {
    GetSample(node->left.get(), p, pos, pdf);
  } else {
    GetSample(node->right.get(), p - node->left->area, pos, pdf);
  }
}

void BVHAccel::Sample(Intersection& pos, float& pdf) {
  if (!m_root || m_root->area <= 0.0F) {
    pos = {};
    pdf = 0.0F;
    return;
  }
  float p = std::sqrt(GetRandomFloat()) * m_root->area;
  GetSample(m_root.get(), p, pos, pdf);
  pdf /= m_root->area;
}
