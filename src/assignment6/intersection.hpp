// Created by LEI XU on 5/16/19.
#pragma once

#include <limits>

#include "material.hpp"
#include "vector.hpp"

class Object;

struct Intersection {
  bool happened = false;
  Vector3f coords;
  Vector3f normal;
  double distance = std::numeric_limits<double>::max();
  Object* obj = nullptr;  // Non-owning; owned by the scene or its mesh.
  Material* m = nullptr;  // Non-owning; owned by the intersected object.
};
