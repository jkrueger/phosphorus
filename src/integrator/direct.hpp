#pragma once

#include "math/ray.hpp"
#include "math/sampling.hpp"
#include "shading.hpp"
#include "thing.hpp"
#include "things/light.hpp"
#include "util/color.hpp"

/**
 * Computes the direct light term on a point in the scene
 *
 */
struct direct_t {
  const uint32_t spd;
  const uint32_t samples;

  direct_t(uint32_t spd)
    : spd(spd*spd), samples(spd*spd) {
  }

  color_t operator()(
      const scene_t& scene
    , const vector_t& wo
    , const shading_info& info) const {
    color_t direct;

    const auto& bxdf = info.bxdf();

    if (emitters.size() > 0 && bxdf->has_distribution()) {
      sample_t         uv[samples];
      sampled_vector_t light_samples[samples];

      // TODO: importance sampling on light sources
      for (const auto& emitter : emitters) {
	// TODO: replace with precomputed samples
	sampling::strategies::stratified_2d(uv, spd);
	emitter->sample(info.p, uv, light_samples, samples); 

	color_t light;
	for (const auto& sample : light_samples) {
	  const auto wi = (sample.sampled - info.p).normalize();

	  if (in_same_hemisphere(wi, info.n) > 0) {
	    if (!scene.occluded(ray_t(info.p + info.n * 0.0001, in), d)) {
	      const auto il = info.b.to_local(wi);
	      const auto ol = info.b.to_local(wo);
	      const auto s  = il.y/(sample.pdf*d*d);

	      light += (emitter->emit() * bxdf->f(il, ol)).scale(s);
	    }
	  }
	}
	direct += light.scale(1.0/samples);
      }
      direct.scale(1.0/scene.lights.size());
    }
    return direct;
  }
};
