#pragma once

#include "math/ray.hpp"
#include "math/sampling.hpp"
#include "shading.hpp"
#include "thing.hpp"
#include "things/light.hpp"
#include "util/color.hpp"

#include "bxdf/lambert.hpp"

struct path_tracer_t {

  static const uint32_t SHADOW_SAMPLES = 256;

  std::vector<light_t::p> emitters;

  color_t trace(const thing_t& scene, const ray_t& ray) const {
    color_t out;

    shading_info_t info;
    if (scene.intersect(ray, info)) {
      out = direct_term(scene, info, ray);
    }

    return out;
  }

  color_t direct_term(const thing_t& scene, const shading_info_t& info, const ray_t& ray) const {
    color_t direct;

    auto out = ray.origin - info.p;
    out.normalize();

    if (emitters.size() > 0) {
      for (auto& emitter : emitters) {
	if (static_cast<const thing_t*>(emitter.get()) !=
	    static_cast<const thing_t*>(info.thing)) {

	  sample_t samples[SHADOW_SAMPLES];
	  emitter->sample(info.p, samples, SHADOW_SAMPLES); 

	  color_t light;
	  for (auto& sample : samples) {
            auto in  = sample.p - info.p;
	    in.normalize();

	    shading_info_t shadow;
	    if (scene.intersect(ray_t(info.p + info.n * 0.0001, in), shadow)) {
	      if (static_cast<const thing_t*>(emitter->thing.get()) ==
		  static_cast<const thing_t*>(shadow.thing)) {
		auto s    = 1.0/(sample.pdf*shadow.d*shadow.d);
		auto bxdf = info.bxdf();

		light += (emitter->emit * bxdf->f(in, out, info.n)).scale(s);
	      }
	    }
	  }
	  direct += light.scale(1.0/SHADOW_SAMPLES);
	}
      }
      direct.scale(1.0/emitters.size());
    }

    return direct;
  }

  color_t indirect_term(const shading_info_t& info) const {
    return {0, 0, 0};
  }
};
