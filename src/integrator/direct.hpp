#pragma once

#include "math/ray.hpp"
#include "math/sampling.hpp"
#include "shading.hpp"
#include "thing.hpp"
#include "things/scene.hpp"
#include "util/color.hpp"

struct direct_t {
  const uint32_t spd;
  const uint32_t samples;

  sample_t uv[9];

  direct_t(uint32_t spd)
    : spd(spd)
    , samples(spd*spd) {
    sampling::strategies::stratified_2d(uv, spd);
  }

  template<typename Scene>
  inline color_t li(
      const Scene& scene
    , const vector_t& wo
    , const material_t::p& m 
    , const shading_info_t* info,
    , uint32_t num) const {
    color_t r;

    const auto bxdf = info.bxdf();

    if (scene.lights.size() > 0 && bxdf->has_distribution()) {
      sampled_vector_t light_samples[samples];

      // TODO: importance sampling on light sources
      for (const auto& emitter : scene.lights) {
	// TODO: replace with precomputed samples
	emitter->sample(info.p, uv, light_samples, samples); 

	color_t light;
	for (const auto& sample : light_samples) {
	  auto wi = sample.sampled - info.p;
	  const auto d  = wi.length();

	  wi.normalize();

	  if (in_same_hemisphere(wi, info.n) &&
	      !scene.occluded(info.ray(wi), d)) {
	    const auto il = info.b.to_local(wi);
	    const auto ol = info.b.to_local(wo);
	    const auto s  = il.y/(sample.pdf*d*d);

	    light += (emitter->emit() * bxdf->f(il, ol)).scale(s);
	  }
	}
	r += light.scale(1.0/samples);
      }
    }
    return r * (1.0/scene.lights.size());
  }
};
