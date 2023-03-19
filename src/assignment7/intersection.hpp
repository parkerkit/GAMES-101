// Created by LEI XU on 5/16/19.
#pragma once

#include <limits>

#include "material.hpp"
#include "vector.hpp"

class Object;

struct Intersection {
  bool happened = false;
  Vector3f coords;
  Vector3f tcoords;
  Vector3f normal;
  Vector3f emit;
  double distance = std::numeric_limits<double>::max();
  Object* obj = nullptr;  // Non-owning; owned by the scene or its mesh.
  Material* m = nullptr;  // Non-owning; keep the referenced material alive while in use.
};
