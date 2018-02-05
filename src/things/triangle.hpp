#pragma once

#include "math/aabb.hpp"

#include <memory>

struct mesh_t;
struct shading_info_t;

struct triangle_t {
  typedef std::shared_ptr<triangle_t> p;

  const mesh_t* mesh;
  uint32_t      id;
  // TODO: precomputed values

  triangle_t(const mesh_t* m, int id);

  // bool intersect(const ray_t& ray, shading_info_t& info) const;

  aabb_t bounds() const;

  vector_t v0() const;
  vector_t v1() const;
  vector_t v2() const;
};
