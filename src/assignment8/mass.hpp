#pragma once

#include "CGL/vector2D.h"

namespace assignment8 {

struct Mass {
  Mass(const CGL::Vector2D& initial_position, float node_mass, bool is_pinned)
      : mass(node_mass),
        pinned(is_pinned),
        start_position(initial_position),
        position(initial_position),
        last_position(initial_position) {}

  float mass;
  bool pinned;
  CGL::Vector2D start_position;
  CGL::Vector2D position;
  // Explicit Verlet integration.
  CGL::Vector2D last_position;
  double last_position_time_step = 0.0;
  // Explicit Euler integration.
  CGL::Vector2D velocity;
  CGL::Vector2D forces;
};

}  // namespace assignment8
