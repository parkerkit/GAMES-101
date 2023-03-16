#pragma once
#include "scene.hpp"

struct HitPayload {
  float t_near = 0;
  uint32_t index = 0;
  Vector2f uv;
  const Object* hit_obj = nullptr;
};

class Renderer {
public:
  void Render(const Scene& scene);
};
