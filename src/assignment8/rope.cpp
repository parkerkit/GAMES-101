#include "rope.hpp"

#include <cmath>
#include <cstddef>
#include <stdexcept>

namespace assignment8 {

Rope::Rope(const CGL::Vector2D& start, const CGL::Vector2D& end, int num_nodes, float node_mass, float k,
           const std::vector<int>& pinned_nodes) {
  if (num_nodes < 2 || !std::isfinite(node_mass) || node_mass <= 0.0F || !std::isfinite(k) || k < 0.0F ||
      !std::isfinite(start.x) || !std::isfinite(start.y) || !std::isfinite(end.x) || !std::isfinite(end.y)) {
    throw std::invalid_argument(
      "A rope needs at least two nodes, positive mass, nonnegative stiffness and finite endpoints");
  }

  const auto count = static_cast<std::size_t>(num_nodes);
  masses.reserve(count);
  springs.reserve(count - 1);
  for (std::size_t i = 0; i < count; ++i) {
    // 包含两个端点，因此间隔数量是 num_nodes - 1；初始速度和合力由 Mass 默认置零。
    const double t = static_cast<double>(i) / static_cast<double>(count - 1);
    const CGL::Vector2D position = start + (end - start) * t;
    masses.push_back(std::make_unique<Mass>(position, node_mass, false));
    if (i > 0) {
      // 弹簧只引用质点地址，质点及弹簧均由绳子的 unique_ptr 容器管理。
      springs.push_back(std::make_unique<Spring>(masses[i - 1].get(), masses[i].get(), k));
    }
  }
  for (int index : pinned_nodes) {
    if (index < 0 || index >= num_nodes) {
      throw std::out_of_range("Pinned node index is outside the rope");
    }
    masses[static_cast<std::size_t>(index)]->pinned = true;
  }
}

void Rope::SimulateEuler(float delta_t, const CGL::Vector2D& gravity) {
  if (!std::isfinite(delta_t) || delta_t <= 0.0F) {
    throw std::invalid_argument("Simulation timestep must be positive and finite");
  }

  constexpr double length_epsilon = 1e-12;
  for (const auto& spring : springs) {
    const CGL::Vector2D displacement = spring->m2->position - spring->m1->position;
    const double length = displacement.norm();
    if (length <= length_epsilon) {
      continue;
    }
    // 伸长时 m1 被拉向 m2；压缩时方向相反。两端受力大小相等、方向相反。
    const CGL::Vector2D force = displacement * (spring->k * (length - spring->rest_length) / length);
    spring->m1->forces += force;
    spring->m2->forces -= force;
  }

  constexpr double damping_coefficient = 0.01;
  for (const auto& mass : masses) {
    if (!mass->pinned) {
      // gravity 是加速度，先乘质量得到重力；全局阻尼力 F_d = -c * v。
      mass->forces += gravity * mass->mass - mass->velocity * damping_coefficient;
      const CGL::Vector2D acceleration = mass->forces / mass->mass;

      // 显式 Euler 对比：启用下一行，并注释下面使用新速度更新位置的语句。
      // mass->position += mass->velocity * delta_t;
      mass->velocity += acceleration * delta_t;
      // 半隐式 Euler：先更新速度，再用 v(t + dt) 更新位置。
      mass->position += mass->velocity * delta_t;
    } else {
      mass->velocity = CGL::Vector2D(0, 0);
      mass->last_position = mass->position;
    }
    // 包括固定点在内，每个子步都清空合力，避免重复累加上一步的力。
    mass->forces = CGL::Vector2D(0, 0);
  }
}

void Rope::SimulateVerlet(float delta_t, const CGL::Vector2D& gravity) {
  if (!std::isfinite(delta_t) || delta_t <= 0.0F) {
    throw std::invalid_argument("Simulation timestep must be positive and finite");
  }

  constexpr double damping_factor = 0.00005;
  const double time_step = delta_t;
  for (const auto& mass : masses) {
    if (!mass->pinned) {
      const CGL::Vector2D previous_position = mass->position;
      const CGL::Vector2D acceleration = gravity + mass->forces / mass->mass;
      // 历史位置差对应上一子步；首次更新尚无历史步长，按当前步长初始化。
      const double previous_time_step = mass->last_position_time_step > 0.0 ? mass->last_position_time_step : time_step;
      const double step_ratio = time_step / previous_time_step;
      // 非等步长 Verlet：惯性位移乘 dt_new / dt_old，加速度项为 a * dt_new * (dt_old + dt_new) / 2。
      // 步长相同时退化为原来的 a * dt^2，仍按作业要求对惯性位移施加全局阻尼。
      const double acceleration_scale = 0.5 * time_step * (previous_time_step + time_step);
      mass->position += (mass->position - mass->last_position) * ((1.0 - damping_factor) * step_ratio) +
                        acceleration * acceleration_scale;
      mass->last_position = previous_position;
    } else {
      mass->velocity = CGL::Vector2D(0, 0);
      mass->last_position = mass->position;
    }
    mass->last_position_time_step = time_step;
    mass->forces = CGL::Vector2D(0, 0);
  }

  // 按 PDF 的无限刚度模型处理距离约束，不再叠加 Hooke 弹簧力。
  // 必须先预测位置再修正；多轮投影减少修正相邻弹簧时重新引入的长度误差。
  constexpr int constraint_iterations = 8;
  constexpr double length_epsilon = 1e-12;
  for (int iteration = 0; iteration < constraint_iterations; ++iteration) {
    for (const auto& spring : springs) {
      const double inverse_mass1 = spring->m1->pinned ? 0.0 : 1.0 / spring->m1->mass;
      const double inverse_mass2 = spring->m2->pinned ? 0.0 : 1.0 / spring->m2->mass;
      const double weight_sum = inverse_mass1 + inverse_mass2;
      if (weight_sum == 0.0) {
        continue;
      }
      const CGL::Vector2D displacement = spring->m2->position - spring->m1->position;
      const double length = displacement.norm();
      if (length <= length_epsilon) {
        continue;
      }
      const CGL::Vector2D correction = displacement * ((length - spring->rest_length) / length);
      // 等质量且都可动时各修正一半；仅一端可动时由该端承担全部修正。
      if (!spring->m1->pinned) {
        spring->m1->position += correction * (inverse_mass1 / weight_sum);
      }
      if (!spring->m2->pinned) {
        spring->m2->position -= correction * (inverse_mass2 / weight_sum);
      }
    }
  }
}

}  // namespace assignment8
