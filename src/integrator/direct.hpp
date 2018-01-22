#pragma once

#include "math/ray.hpp"
#include "math/sampling.hpp"
#include "shading.hpp"
#include "thing.hpp"
#include "things/light.hpp"
#include "util/color.hpp"

/**
 * Computes an estimate of the direct light term on a point
 * in the scene
 *
 */
struct direct_t {
  const uint32_t samples;

  direct_t(uint32_t spd)
    : samples(spd*spd) {
  }

  color_t operator()(
      const scene_t& scene
    , const vector_t& wi
    , const shading_info& info) const {
    color_t direct;

    const auto& bxdf = info.bxdf();

    if (emitters.size() > 0 && bxdf->has_distribution()) {
      auto out  = ray.origin - info.p;
      out.normalize();

      sample_t         uv[samples];
      sampled_vector_t light_samples[samples];

      for (auto& emitter : emitters) {
	sampling::strategies::stratified_2d(uv, 3);
	emitter->sample(info.p, uv, light_samples, SHADOW_SAMPLES); 

	color_t light;
	for (auto& sample : light_samples) {
	  auto in  = sample.sampled - info.p;
	  in.normalize();
	  
	  if (dot(in, info.n) > 0) {
	    shading_info_t shadow;
	    if (scene.intersect(ray_t(info.p + info.n * 0.0001, in), shadow)) {
	      auto il = info.b.to_local(in);
	      auto ol = info.b.to_local(out);
	      auto s  = il.y/(sample.pdf*shadow.d*shadow.d);
	      light += (emitter->emit() * bxdf->f(il, ol)).scale(s);
	    }
	  }
	}
	direct += light.scale(1.0/SHADOW_SAMPLES);
      }
      direct.scale(1.0/emitters.size());
    }
    return direct;
  }
};
