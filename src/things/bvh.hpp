#pragma once

#include "thing.hpp"

#include <memory>

struct bvh_t : public thing_t {
  typedef std::shared_ptr<bvh_t> p;
  
  struct impl_t;

  bvh_t();
  
  void build(const std::vector<thing_t::p>& things);
  
  bool intersect(const ray_t&, shading_info_t&) const;

  aabb_t bounds() const
  { return aabb_t(); }

  void sample(
    const vector_t&,
    const sample_t*,
    sampled_vector_t*,
    uint32_t) const
  {}

  std::shared_ptr<impl_t> impl;
};
