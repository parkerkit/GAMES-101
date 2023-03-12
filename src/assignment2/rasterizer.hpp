//
// Created by goksu on 4/6/19.
//

#pragma once

#include <Eigen/Core>
#include <map>
#include <vector>

#include "triangle.hpp"

namespace rst {
enum class Buffers { Color = 1, Depth = 2 };

inline Buffers operator|(Buffers a, Buffers b) {
  return static_cast<Buffers>(static_cast<int>(a) | static_cast<int>(b));
}

inline Buffers operator&(Buffers a, Buffers b) {
  return static_cast<Buffers>(static_cast<int>(a) & static_cast<int>(b));
}

enum class Primitive { Line, Triangle };

/*
 * For the curious : The Draw function takes two buffer id's as its arguments. These two structs
 * make sure that if you mix up with their orders, the compiler won't compile it.
 * Aka : Type safety
 * */
struct PosBufId {
  int pos_id = 0;
};

struct IndBufId {
  int ind_id = 0;
};

struct ColBufId {
  int col_id = 0;
};

class Rasterizer {
public:
  Rasterizer(int w, int h);
  PosBufId LoadPositions(const std::vector<Eigen::Vector3f>& positions);
  IndBufId LoadIndices(const std::vector<Eigen::Vector3i>& indices);
  ColBufId LoadColors(const std::vector<Eigen::Vector3f>& cols);

  void SetModel(const Eigen::Matrix4f& m);
  void SetView(const Eigen::Matrix4f& v);
  void SetProjection(const Eigen::Matrix4f& p);

  void SetPixel(const Eigen::Vector3f& point, const Eigen::Vector3f& color);

  void Clear(Buffers buff);

  void Draw(PosBufId pos_buffer, IndBufId ind_buffer, ColBufId col_buffer, Primitive type);

  std::vector<Eigen::Vector3f>& FrameBuffer() { return m_frame_buf; }

private:
  void DrawLine(Eigen::Vector3f begin, Eigen::Vector3f end);

  void RasterizeTriangle(const Triangle& t);

  // VERTEX SHADER -> MVP -> Clipping -> /.W -> VIEWPORT -> DRAWLINE/DRAWTRI -> FRAGSHADER

private:
  Eigen::Matrix4f m_model;
  Eigen::Matrix4f m_view;
  Eigen::Matrix4f m_projection;

  std::map<int, std::vector<Eigen::Vector3f>> m_pos_buf;
  std::map<int, std::vector<Eigen::Vector3i>> m_ind_buf;
  std::map<int, std::vector<Eigen::Vector3f>> m_col_buf;

  std::vector<Eigen::Vector3f> m_frame_buf;

  std::vector<float> m_depth_buf;
  int GetIndex(int x, int y);

  int m_width;
  int m_height;

  int m_next_id = 0;
  int GetNextId() { return m_next_id++; }
};
}  // namespace rst
