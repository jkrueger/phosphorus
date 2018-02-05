#pragma once

#include "shading.hpp"
#include "things/triangle.hpp"

#include <memory>

template<typename T>
struct bvh_t {
  typedef std::shared_ptr<bvh_t> p;
  
  struct impl_t;

  bvh_t();

  void build(const std::vector<triangle_t::p>& things);

  bool intersect(segment_t&, float& d) const;

  bool occluded(segment_t& segment, float_t d) const;

  std::shared_ptr<impl_t> impl;
};

typedef bvh_t<triangle_t> mesh_bvh_t;
