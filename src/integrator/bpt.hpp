#pragma once

#include "math/ray.hpp"
#include "math/sampling.hpp"
#include "scene.hpp"
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

    const auto bxdf  = info.bxdf();
    const auto light = scene.sample_light();
    const auto path  = bxdf.sample(info.n, sample, wo);

    // trace light and camera paths and connect paths though and occlusion check
    
    return indirect;
  }
};
