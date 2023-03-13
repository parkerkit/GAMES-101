//
// Created by LEI XU on 4/27/19.
//

#ifndef RASTERIZER_SHADER_H
#define RASTERIZER_SHADER_H
#include <Eigen/Dense>

#include "texture.hpp"

struct FragmentShaderPayload {
  FragmentShaderPayload() = default;

  FragmentShaderPayload(const Eigen::Vector3f& col, const Eigen::Vector3f& nor, const Eigen::Vector2f& tc, Texture* tex)
      : color(col), normal(nor), tex_coords(tc), texture(tex) {}

  Eigen::Vector3f view_pos = Eigen::Vector3f::Zero();
  Eigen::Vector3f color = Eigen::Vector3f::Zero();
  Eigen::Vector3f normal = Eigen::Vector3f::Zero();
  Eigen::Vector2f tex_coords = Eigen::Vector2f::Zero();
  Texture* texture = nullptr;
};

struct VertexShaderPayload {
  Eigen::Vector3f position = Eigen::Vector3f::Zero();
};

#endif  // RASTERIZER_SHADER_H
