#pragma once

#include <memory>
#include <utility>
#include <vector>

#include "light.hpp"
#include "object.hpp"
#include "vector.hpp"

class Scene {
public:
  Scene(int width, int height) : m_width(width), m_height(height) {}

  void Add(std::unique_ptr<Object> object) { m_objects.push_back(std::move(object)); }
  void Add(std::unique_ptr<Light> light) { m_lights.push_back(std::move(light)); }

  [[nodiscard]] int GetWidth() const { return m_width; }
  [[nodiscard]] int GetHeight() const { return m_height; }
  [[nodiscard]] float GetFov() const { return m_fov; }
  [[nodiscard]] const Vector3f& GetBackgroundColor() const { return m_background_color; }
  [[nodiscard]] int GetMaxDepth() const { return m_max_depth; }
  [[nodiscard]] float GetEpsilon() const { return m_epsilon; }
  [[nodiscard]] const std::vector<std::unique_ptr<Object>>& GetObjects() const { return m_objects; }
  [[nodiscard]] const std::vector<std::unique_ptr<Light>>& GetLights() const { return m_lights; }

private:
  int m_width = 1280;
  int m_height = 960;
  float m_fov = 90.0F;
  Vector3f m_background_color{0.235294F, 0.67451F, 0.843137F};
  int m_max_depth = 5;
  float m_epsilon = 0.00001F;
  std::vector<std::unique_ptr<Object>> m_objects;
  std::vector<std::unique_ptr<Light>> m_lights;
};
