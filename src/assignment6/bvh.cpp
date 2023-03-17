#include "bvh.hpp"

#include <algorithm>
#include <cassert>
#include <chrono>
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
    node->left = nullptr;
    node->right = nullptr;
    return node;
  }
  if (objects.size() == 2) {
    node->left = RecursiveBuild(std::vector{objects[0]});
    node->right = RecursiveBuild(std::vector{objects[1]});

    node->bounds = Union(node->left->bounds, node->right->bounds);
    return node;
  }
  {
    Bounds3 centroid_bounds;
    for (auto* object : objects) {
      centroid_bounds = Union(centroid_bounds, object->GetBounds().Centroid());
    }
    int dim = centroid_bounds.MaxExtent();
    switch (dim) {
      case 0:
        std::sort(objects.begin(), objects.end(),
                  [](auto f1, auto f2) { return f1->GetBounds().Centroid().x < f2->GetBounds().Centroid().x; });
        break;
      case 1:
        std::sort(objects.begin(), objects.end(),
                  [](auto f1, auto f2) { return f1->GetBounds().Centroid().y < f2->GetBounds().Centroid().y; });
        break;
      case 2:
        std::sort(objects.begin(), objects.end(),
                  [](auto f1, auto f2) { return f1->GetBounds().Centroid().z < f2->GetBounds().Centroid().z; });
        break;
    }

    auto beginning = objects.begin();
    auto middling = objects.begin() + static_cast<std::ptrdiff_t>(objects.size() / 2);
    auto ending = objects.end();

    auto leftshapes = std::vector<Object*>(beginning, middling);
    auto rightshapes = std::vector<Object*>(middling, ending);

    assert(objects.size() == (leftshapes.size() + rightshapes.size()));

    node->left = RecursiveBuild(leftshapes);
    node->right = RecursiveBuild(rightshapes);

    node->bounds = Union(node->left->bounds, node->right->bounds);
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
  // TODO: Traverse the BVH to find the closest intersection.
  // Child nodes are owned by unique_ptr; use node->left.get() / node->right.get() when recursing.
  return {};
}
