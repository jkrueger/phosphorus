#pragma once

#include "math/ray.hpp"
#include "math/sampling.hpp"
#include "shading.hpp"
#include "util/color.hpp"

/**
 * Birectional path tracing integrator
 * 
 */
struct bpt_t {
  const uint32_t max_depth;

  bpd_t(uint32_t d)
    : max_depth(d)
  {}

  color_t operator()(
      const scene_t& scene
    , const vector_t& wi
    , const shading_info_t& info) const {
    color_t indirect;

    // generate light and camera path segments and connect
    
    return indirect;
  }
};
