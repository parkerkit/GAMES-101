#pragma once

#include <memory>
#include <utility>
#include <vector>

#include "CGL/vector2D.h"
#include "mass.hpp"
#include "spring.hpp"

namespace assignment8 {

class Rope {
public:
  Rope(std::vector<std::unique_ptr<Mass>> initial_masses, std::vector<std::unique_ptr<Spring>> initial_springs)
      : masses(std::move(initial_masses)), springs(std::move(initial_springs)) {}
  Rope(const CGL::Vector2D& start, const CGL::Vector2D& end, int num_nodes, float node_mass, float k,
       const std::vector<int>& pinned_nodes);

  void SimulateVerlet(float delta_t, const CGL::Vector2D& gravity);
  void SimulateEuler(float delta_t, const CGL::Vector2D& gravity);

  // Springs are destroyed before their endpoints.
  std::vector<std::unique_ptr<Mass>> masses;
  std::vector<std::unique_ptr<Spring>> springs;
};

}  // namespace assignment8
