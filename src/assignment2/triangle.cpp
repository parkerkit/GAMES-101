//
// Created by LEI XU on 4/11/19.
//

#include "triangle.hpp"

#include <algorithm>
#include <array>
#include <stdexcept>

Triangle::Triangle() {
  v[0] << 0, 0, 0;
  v[1] << 0, 0, 0;
  v[2] << 0, 0, 0;

  color[0] << 0.0F, 0.0F, 0.0F;
  color[1] << 0.0F, 0.0F, 0.0F;
  color[2] << 0.0F, 0.0F, 0.0F;

  tex_coords[0] << 0.0F, 0.0F;
  tex_coords[1] << 0.0F, 0.0F;
  tex_coords[2] << 0.0F, 0.0F;
}

void Triangle::SetVertex(int ind, Eigen::Vector3f ver) { v[ind] = ver; }
void Triangle::SetNormal(int ind, Eigen::Vector3f n) { normal[ind] = n; }
void Triangle::SetColor(int ind, float r, float g, float b) {
  if ((r < 0.0F) || (r > 255.F) || (g < 0.0F) || (g > 255.F) || (b < 0.0F) || (b > 255.F)) {
    throw std::runtime_error("Invalid color values");
  }

  color[ind] = Eigen::Vector3f(r / 255.0F, g / 255.0F, b / 255.0F);
}
void Triangle::SetTexCoord(int ind, float s, float t) { tex_coords[ind] = Eigen::Vector2f(s, t); }

std::array<Eigen::Vector4f, 3> Triangle::ToVector4() const {
  std::array<Eigen::Vector4f, 3> res;
  std::transform(std::begin(v), std::end(v), res.begin(),
                 [](auto& vec) { return Eigen::Vector4f(vec.x(), vec.y(), vec.z(), 1.F); });
  return res;
}
