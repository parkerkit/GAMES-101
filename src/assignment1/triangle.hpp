//
// Created by LEI XU on 4/11/19.
//

#ifndef RASTERIZER_TRIANGLE_H
#define RASTERIZER_TRIANGLE_H

#include <Eigen/Core>
#include <array>

class Triangle {
public:
  std::array<Eigen::Vector3f, 3> v; /*the original coordinates of the triangle, v0, v1, v2 in
                    counter clockwise order*/
  /*Per vertex values*/
  std::array<Eigen::Vector3f, 3> color;       // color at each vertex;
  std::array<Eigen::Vector2f, 3> tex_coords;  // texture u,v
  std::array<Eigen::Vector3f, 3> normal;      // normal vector for each vertex

  // Texture *tex;
  Triangle();

  Eigen::Vector3f A() const { return v[0]; }
  Eigen::Vector3f B() const { return v[1]; }
  Eigen::Vector3f C() const { return v[2]; }

  void SetVertex(int ind, Eigen::Vector3f ver);      /*set i-th vertex coordinates */
  void SetNormal(int ind, Eigen::Vector3f n);        /*set i-th vertex normal vector*/
  void SetColor(int ind, float r, float g, float b); /*set i-th vertex color*/
  void SetTexCoord(int ind, float s, float t);       /*set i-th vertex texture coordinate*/
  std::array<Eigen::Vector4f, 3> ToVector4() const;
};

#endif  // RASTERIZER_TRIANGLE_H
