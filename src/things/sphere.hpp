#pragma once

#include "precision.hpp"
#include "thing.hpp"

#include "math/sampling.hpp"
#include "math/vector.hpp"

struct sphere_t : public shadable_t {
  float_t   radius;
  float_t   radius2;

  sphere_t(const vector_t& c, float_t r, const material_t::p& m);

  bool intersect(const ray_t& ray, shading_info_t& info) const;

  void shading_parameters(shading_info_t& info, const vector_t& p) const;

  void sample(
    const vector_t& p,
    const sample_t* samples,
    sampled_vector_t* sampled,
    uint32_t num) const;

  aabb_t bounds() const;
};
