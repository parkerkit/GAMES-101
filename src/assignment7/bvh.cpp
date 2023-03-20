#include "bvh.hpp"

#include <algorithm>
#include <array>
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
  // 为满足作业 PDF 的均匀面积采样要求，按累计面积均匀选取图元。
  // 与原框架不同：去掉这里的 sqrt，否则实际采样分布与返回的采样密度 1 / area 不一致。
  float p = GetRandomFloat() * m_root->area;
  GetSample(m_root.get(), p, pos, pdf);
  pdf /= m_root->area;
}
