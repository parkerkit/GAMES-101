#pragma once

#include <cstddef>
#include <memory>
#include <string>

#include "CGL/renderer.h"
#include "CGL/vector2D.h"
#include "rope.hpp"

namespace assignment8 {

struct AppConfig {
  float mass = 1.0F;
  float ks = 100.0F;
  int steps_per_frame = 64;
  CGL::Vector2D gravity{0, -1};
};

class Application : public CGL::Renderer {
public:
  explicit Application(const AppConfig& config);
  Application(const Application&) = delete;
  Application& operator=(const Application&) = delete;
  Application(Application&&) = delete;
  Application& operator=(Application&&) = delete;
  ~Application() override = default;

  // CGL virtual interfaces keep their original names.
  void init() override;
  void render() override;
  void resize(std::size_t w, std::size_t h) override;
  std::string name() override;
  std::string info() override;
  void key_event(char key) override;

private:
  AppConfig m_config;
  std::unique_ptr<Rope> m_rope_euler;
  std::unique_ptr<Rope> m_rope_verlet;
  std::size_t m_screen_width = 0;
  std::size_t m_screen_height = 0;
};

}  // namespace assignment8
