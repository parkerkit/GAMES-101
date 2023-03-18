#include "bvh.hpp"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <limits>
#include <utility>

namespace {

// Sweep SAH：在三个轴上按质心排序，评估每个能将图元分成非空两组的候选切分。
// 返回左侧图元数量；0 表示无法找到有效 SAH 划分，由调用方回退到中点划分。
std::size_t PartitionSah(std::vector<Object*>& objects, int& split_axis) {
  Bounds3 parent_bounds;
  for (auto* object : objects) {
    parent_bounds = Union(parent_bounds, object->GetBounds());
  }
  const double parent_area = parent_bounds.SurfaceArea();
  if (!std::isfinite(parent_area) || parent_area <= 0.0) {
    return 0;
  }

  const std::size_t count = objects.size();
  double best_cost = std::numeric_limits<double>::infinity();
  int best_axis = 0;
  std::size_t best_split = 0;
  std::size_t best_balance = 0;
  std::vector<Bounds3> suffix_bounds(count);
  for (int axis = 0; axis < 3; ++axis) {
    std::sort(objects.begin(), objects.end(), [axis](Object* a, Object* b) {
      return a->GetBounds().Centroid()[axis] < b->GetBounds().Centroid()[axis];
    });

    // 后缀包围盒保存 [i, count) 的范围，前缀包围盒在下一次扫描中逐步累积。
    // 每个轴排序后只需线性扫描，避免为每个候选切分重新遍历全部图元。
    suffix_bounds.back() = objects.back()->GetBounds();
    for (std::size_t i = count - 1; i > 0; --i) {
      suffix_bounds[i - 1] = Union(objects[i - 1]->GetBounds(), suffix_bounds[i]);
    }
    Bounds3 left_bounds;
    for (std::size_t i = 1; i < count; ++i) {
      left_bounds = Union(left_bounds, objects[i - 1]->GetBounds());
      // 质心相同时不存在将这两个图元分开的轴向切分位置。
      if (objects[i - 1]->GetBounds().Centroid()[axis] == objects[i]->GetBounds().Centroid()[axis]) {
        continue;
      }
      // SAH = Ct + Ci * (A_left * N_left + A_right * N_right) / A_parent。
      // 比较同一父节点的切分时，Ct、Ci 和 A_parent 都相同，可省略这些常量。
      const double cost = left_bounds.SurfaceArea() * static_cast<double>(i) +
                          suffix_bounds[i].SurfaceArea() * static_cast<double>(count - i);
      const std::size_t balance = std::min(i, count - i);
      // 成本相同时优先更均衡的划分，避免无收益的单侧长链。
      if (std::isfinite(cost) && (cost < best_cost || (cost == best_cost && balance > best_balance))) {
        best_cost = cost;
        best_axis = axis;
        best_split = i;
        best_balance = balance;
      }
    }
  }
  if (best_split != 0) {
    split_axis = best_axis;
    std::sort(objects.begin(), objects.end(), [best_axis](Object* a, Object* b) {
      return a->GetBounds().Centroid()[best_axis] < b->GetBounds().Centroid()[best_axis];
    });
  }
  return best_split;
}

}  // namespace

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
  const char* method_name = m_split_method == SplitMethod::Sah ? "SAH" : "Naive";
  std::cout << "BVH generation complete (" << method_name << "): " << elapsed.count() << " ms\n";
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
    std::size_t split_index = m_split_method == SplitMethod::Sah ? PartitionSah(objects, dim) : 0;
    if (split_index == 0) {
      // Naive 模式，或 SAH 遇到质心重合、退化包围盒时，使用最长质心轴的中点划分。
      std::sort(objects.begin(), objects.end(), [dim](Object* a, Object* b) {
        return a->GetBounds().Centroid()[dim] < b->GetBounds().Centroid()[dim];
      });
      split_index = objects.size() / 2;
    }
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
