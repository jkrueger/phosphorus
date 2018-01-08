#pragma once

#include "thing.hpp"

struct plane_t : public shadable_t {
  vector_t n;

  plane_t(const vector_t& origin, const vector_t& n, const material_t::p& m);

  bool intersect(const ray_t& ray, shading_info_t& info) const;

  void shading_parameters(shading_info_t& info, const vector_t& p) const;

  void sample(
    const vector_t& p,
    const sample_t* samples,
    sampled_vector_t* out,
    uint32_t num) const;

  aabb_t bounds() const;
};
