#pragma once

#include "thing.hpp"

struct mesh_t;

struct triangle_t : public thing_t {
  const mesh_t* mesh;
  uint32_t a, b, c;
  // TODO: precomputed values

  bool intersect(const ray_t& ray, shading_info_t& info) const;

  void sample(const vector_t&, const sample_t&, sampled_vector_t&, uint32_t) const;

  void shading_parameters(shading_info&, const vector_t&, double u, double v) const;
};

struct mesh_t : public shadable_t {
  std::vector<vector_t>   vertices;
  std::vector<normal_t>   normals;
  std::vector<triangle_t> triangles;

  aabb_t _bounds;

  mesh_t(const vector_t& p)
    : thing_t(p)
  {}

  void tesselate(std::vector_t<triangle_t>&) const;

  bool intersect(const ray_t&, shading_info_t&) const;

  void sample(const vector_t&, const sample_t&, sampled_vector_t&, uint32_t) const;
  
  aabb_t bounds() const
  { return _bounds; }
};
