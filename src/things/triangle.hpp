#pragma once

#include "math/aabb.hpp"

#include <memory>

struct mesh_t;
struct shading_info_t;

struct triangle_t {
  typedef std::shared_ptr<triangle_t> p;
  
  const mesh_t* mesh;
  uint32_t a, b, c;
  // TODO: precomputed values

  triangle_t(const mesh_t* m, int a, int b, int c);

  // bool intersect(const ray_t& ray, shading_info_t& info) const;

  void shading_parameters(shading_info_t&, const vector_t&, float_t u, float_t v) const;

  aabb_t bounds() const;

  vector_t v0() const;
  vector_t v1() const;
  vector_t v2() const;
};
