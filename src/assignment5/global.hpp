#pragma once

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <numbers>
#include <random>
#include <utility>

inline constexpr float k_pi = std::numbers::pi_v<float>;

inline constexpr float k_infinity = std::numeric_limits<float>::max();

inline float Clamp(const float& lo, const float& hi, const float& v) { return std::max(lo, std::min(hi, v)); }

inline bool SolveQuadratic(const float& a, const float& b, const float& c, float& x0, float& x1) {
  float discr = b * b - 4 * a * c;
  if (discr < 0) {
    return false;
  }
  if (discr == 0) {
    x0 = x1 = -0.5F * b / a;
  } else {
    float q = (b > 0) ? -0.5F * (b + std::sqrt(discr)) : -0.5F * (b - std::sqrt(discr));
    x0 = q / a;
    x1 = c / q;
  }
  if (x0 > x1) {
    std::swap(x0, x1);
  }
  return true;
}

enum MaterialType { DiffuseAndGlossy, ReflectionAndRefraction, Reflection };

inline float GetRandomFloat() {
  std::random_device dev;
  std::mt19937 rng(dev());
  std::uniform_real_distribution<float> dist(0.0F, 1.0F);  // Uniform distribution in [0, 1).

  return dist(rng);
}

inline void UpdateProgress(float progress) {
  int bar_width = 70;

  std::cout << "[";
  int pos = static_cast<int>(static_cast<float>(bar_width) * progress);
  for (int i = 0; i < bar_width; ++i) {
    if (i < pos) {
      std::cout << "=";
    } else if (i == pos) {
      std::cout << ">";
    } else {
      std::cout << " ";
    }
  }
  std::cout << "] " << static_cast<int>(progress * 100.0F) << " %\r";
  std::cout.flush();
}
