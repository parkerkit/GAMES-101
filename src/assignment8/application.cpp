#include "application.hpp"

#include <array>
#include <limits>
#include <memory>
#include <sstream>
#include <vector>

#include "GL/glew.h"

namespace assignment8 {

Application::Application(const AppConfig& config) : m_config(config) {}

void Application::init() {
  // Enable anti-aliasing and circular points.
  glEnable(GL_LINE_SMOOTH);
  glEnable(GL_POLYGON_SMOOTH);
  glEnable(GL_POINT_SMOOTH);
  glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
  glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
  glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);

  glPointSize(8.0F);
  glLineWidth(4.0F);
  glColor3f(1.0F, 1.0F, 1.0F);

  // 按作业要求增加节点数量，便于观察两种模拟方式的运动差异。
  constexpr int num_nodes = 16;
  const std::vector<int> pinned_nodes{0};
  m_rope_euler = std::make_unique<Rope>(CGL::Vector2D(0, 200), CGL::Vector2D(-400, 200), num_nodes, m_config.mass,
                                        m_config.ks, pinned_nodes);
  m_rope_verlet = std::make_unique<Rope>(CGL::Vector2D(0, 200), CGL::Vector2D(-400, 200), num_nodes, m_config.mass,
                                         m_config.ks, pinned_nodes);
}

void Application::render() {
  const float delta_t = 1.0F / static_cast<float>(m_config.steps_per_frame);
  for (int i = 0; i < m_config.steps_per_frame; ++i) {
    m_rope_euler->SimulateEuler(delta_t, m_config.gravity);
    m_rope_verlet->SimulateVerlet(delta_t, m_config.gravity);
  }

  const std::array<Rope*, 2> ropes{m_rope_euler.get(), m_rope_verlet.get()};
  for (std::size_t i = 0; i < ropes.size(); ++i) {
    if (i == 0) {
      glColor3f(0.0F, 0.0F, 1.0F);
    } else {
      glColor3f(0.0F, 1.0F, 0.0F);
    }
    const Rope* rope = ropes[i];

    glBegin(GL_POINTS);
    for (const auto& mass : rope->masses) {
      const CGL::Vector2D& point = mass->position;
      glVertex2d(point.x, point.y);
    }
    glEnd();

    glBegin(GL_LINES);
    for (const auto& spring : rope->springs) {
      const CGL::Vector2D& p1 = spring->m1->position;
      const CGL::Vector2D& p2 = spring->m2->position;
      glVertex2d(p1.x, p1.y);
      glVertex2d(p2.x, p2.y);
    }
    glEnd();
    glFlush();
  }
}

void Application::resize(std::size_t w, std::size_t h) {
  m_screen_width = w;
  m_screen_height = h;
  const double half_width = static_cast<double>(m_screen_width) / 2.0;
  const double half_height = static_cast<double>(m_screen_height) / 2.0;

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-half_width, half_width, -half_height, half_height, 1, 0);
}

void Application::key_event(char key) {
  switch (key) {
    case '-':
      if (m_config.steps_per_frame > 1) {
        m_config.steps_per_frame /= 2;
      }
      break;
    case '=':
      if (m_config.steps_per_frame <= std::numeric_limits<int>::max() / 2) {
        m_config.steps_per_frame *= 2;
      }
      break;
    default:
      break;
  }
}

std::string Application::name() { return "Rope Simulator"; }

std::string Application::info() {
  std::ostringstream steps;
  steps << "Steps per frame: " << m_config.steps_per_frame;
  return steps.str();
}

}  // namespace assignment8
