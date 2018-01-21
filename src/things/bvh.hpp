#pragma once

#include "triangle.hpp"

#include <memory>

template<typename T>
struct bvh_t {
  typedef std::shared_ptr<bvh_t> p;
  
  struct impl_t;

  bvh_t();

  void build(const std::vector<triangle_t::p>& things);

  bool intersect(const ray_t&, shading_info_t&) const;

  bool occluded(const ray_t& ray, float_t d) const;

  std::shared_ptr<impl_t> impl;
};

typedef bvh_t<triangle_t> mesh_bvh_t;
