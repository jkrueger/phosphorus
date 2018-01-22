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

    const auto light_sample = uniform_sample_hemisphere();
    const auto path_sample  = bxdf.sample(info.n, sample, wo);

    shading_info_t light_info;
    if (!scene.trace(ray_t(light, light_sample), light_info)) {
      // abort
    }

    shading_info_t camera_info;
    if (!scene.trace(ray_t(info.p, path_sample), camera_info)) {
      // abort
    }

    // connect paths
    
    return indirect;
  }
};
