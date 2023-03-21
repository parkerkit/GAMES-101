#pragma once

#include "mass.hpp"

namespace assignment8 {

struct Spring {
  Spring(Mass* a, Mass* b, float stiffness)
      : k(stiffness), rest_length((a->position - b->position).norm()), m1(a), m2(b) {}

  float k;
  double rest_length;
  // Non-owning endpoints; the rope owns the masses.
  Mass* m1;
  Mass* m2;
};

}  // namespace assignment8
