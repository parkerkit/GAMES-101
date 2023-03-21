#include "rope.hpp"

namespace assignment8 {

Rope::Rope([[maybe_unused]] const CGL::Vector2D& start, [[maybe_unused]] const CGL::Vector2D& end,
           [[maybe_unused]] int num_nodes, [[maybe_unused]] float node_mass, [[maybe_unused]] float k,
           [[maybe_unused]] const std::vector<int>& pinned_nodes) {
  // TODO (Part 1): Create a rope starting at `start`, ending at `end`, and containing `num_nodes` nodes.
  // masses 和 springs 持有 unique_ptr，可用 std::make_unique 创建节点和弹簧。

  // Comment-in this part when you implement the constructor.
  // for (int index : pinned_nodes) {
  //   masses[index]->pinned = true;
  // }
}

void Rope::SimulateEuler([[maybe_unused]] float delta_t, [[maybe_unused]] const CGL::Vector2D& gravity) {
  for ([[maybe_unused]] const auto& spring : springs) {
    // TODO (Part 2): Use Hooke's law to calculate the force on a node.
  }

  for (const auto& mass : masses) {
    if (!mass->pinned) {
      // TODO (Part 2): Add the force due to gravity, then compute the new velocity and position.

      // TODO (Part 2): Add global damping.
    }

    // Reset all forces on each mass.
    mass->forces = CGL::Vector2D(0, 0);
  }
}

void Rope::SimulateVerlet([[maybe_unused]] float delta_t, [[maybe_unused]] const CGL::Vector2D& gravity) {
  for ([[maybe_unused]] const auto& spring : springs) {
    // TODO (Part 3): Simulate one timestep of the rope using explicit Verlet (solving constraints).
  }

  for (const auto& mass : masses) {
    if (!mass->pinned) {
      [[maybe_unused]] CGL::Vector2D temp_position = mass->position;
      // TODO (Part 3.1): Set the new position of the rope mass.

      // TODO (Part 4): Add global Verlet damping.
    }
  }
}

}  // namespace assignment8
