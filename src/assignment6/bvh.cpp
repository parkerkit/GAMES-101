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

Intersection BVHAccel::GetIntersection(const BVHBuildNode* node, const Ray& ray) const {
  if (node == nullptr) {
    return {};
  }
  // 与 Bounds3::IntersectP 的课程约定一致：方向分量为正时记为 1。
  const std::array<int, 3> dir_is_neg{ray.direction.x > 0 ? 1 : 0, ray.direction.y > 0 ? 1 : 0,
                                      ray.direction.z > 0 ? 1 : 0};
  // 未命中节点包围盒，就不可能命中其内部图元，整棵子树可直接剪枝。
  if (!node->bounds.IntersectP(ray, ray.direction_inv, dir_is_neg)) {
    return {};
  }
  // 当前构建方式中，每个叶节点保存一个图元；具体求交交给图元自身处理。
  // 场景 BVH 的图元也可能是网格，其 GetIntersection 会继续查询网格内部的 BVH。
  if (node->object != nullptr) {
    return node->object->GetIntersection(ray);
  }

  // 包围盒命中只说明“可能相交”，仍需递归查询两侧，不能直接返回第一个命中。
  // 子节点由 unique_ptr 持有，get() 仅取得访问指针，不转移所有权。
  const Intersection left_hit = GetIntersection(node->left.get(), ray);
  const Intersection right_hit = GetIntersection(node->right.get(), ray);
  if (!left_hit.happened) {
    return right_hit;
  }
  // 两侧都命中时比较同一条射线上的参数距离，返回离起点最近的可见表面。
  if (!right_hit.happened || left_hit.distance <= right_hit.distance) {
    return left_hit;
  }
  return right_hit;
}
