//
// Created by LEI XU on 4/11/19.
//

#include "triangle.hpp"

#include <array>
#include <stdexcept>

Triangle::Triangle() {
  normal.fill(Eigen::Vector3f::Zero());
  v[0] << 0, 0, 0, 1;
  v[1] << 0, 0, 0, 1;
  v[2] << 0, 0, 0, 1;

  color[0] << 0.0F, 0.0F, 0.0F;
  color[1] << 0.0F, 0.0F, 0.0F;
  color[2] << 0.0F, 0.0F, 0.0F;

  tex_coords[0] << 0.0F, 0.0F;
  tex_coords[1] << 0.0F, 0.0F;
  tex_coords[2] << 0.0F, 0.0F;
}

void Triangle::SetVertex(int ind, Eigen::Vector4f ver) { v[ind] = ver; }
void Triangle::SetNormal(int ind, Eigen::Vector3f n) { normal[ind] = n; }
void Triangle::SetColor(int ind, float r, float g, float b) {
  if ((r < 0.0F) || (r > 255.F) || (g < 0.0F) || (g > 255.F) || (b < 0.0F) || (b > 255.F)) {
    throw std::out_of_range("Invalid color values");
  }

  color[ind] = Eigen::Vector3f(r / 255.0F, g / 255.0F, b / 255.0F);
}
void Triangle::SetTexCoord(int ind, Eigen::Vector2f uv) { tex_coords[ind] = uv; }

std::array<Eigen::Vector4f, 3> Triangle::ToVector4() const { return v; }

void Triangle::SetNormals(const std::array<Eigen::Vector3f, 3>& normals) {
  normal[0] = normals[0];
  normal[1] = normals[1];
  normal[2] = normals[2];
}

void Triangle::SetColors(const std::array<Eigen::Vector3f, 3>& colors) {
  SetColor(0, colors[0][0], colors[0][1], colors[0][2]);
  SetColor(1, colors[1][0], colors[1][1], colors[1][2]);
  SetColor(2, colors[2][0], colors[2][1], colors[2][2]);
}
