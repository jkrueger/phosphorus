#pragma once

#include "math/ray.hpp"
#include "math/sampling.hpp"
#include "shading.hpp"
#include "thing.hpp"
#include "things/light.hpp"
#include "util/color.hpp"

#include "bxdf/lambert.hpp"

struct path_tracer_t {

  static const uint32_t SHADOW_SAMPLES = 32;
  static const uint8_t  MAX_DEPTH = 0;

  std::vector<light_t::p> emitters;

  color_t trace(const thing_t& scene, const ray_t& ray) const {
    color_t out;
    ray_t   path = ray;
    color_t beta(1.0);

    uint8_t depth = 0;
    while (depth <= MAX_DEPTH) {

      shading_info_t info;
      if (scene.intersect(path, info)) {
	out += beta * direct(scene, info, ray);
	if (depth == 0) {
	  out += info.thing->emissive;
	}
      }
      else {
	// path escaped the scene
	break;
      }

      sample_t next;
      auto r = info.bxdf()->sample(info.b.to_local(-ray.direction), next);

      path = ray_t(info.p + info.n * 0.0001, info.b.to_world(next.p));
      beta = beta * r * std::abs(dot(next.p, info.n)) * (1.0/next.pdf);

      ++depth;
    }
    
    return out;
  }

  color_t direct(const thing_t& scene, shading_info_t& info, const ray_t& ray) const {
    color_t direct;
    auto bxdf = info.bxdf();

    if (emitters.size() > 0 && bxdf->has_distribution()) {
      auto out  = ray.origin - info.p;
      out.normalize();

      for (auto& emitter : emitters) {
	if (static_cast<const thing_t*>(emitter.get()) !=
	    static_cast<const thing_t*>(info.thing)) {

	  sample_t samples[SHADOW_SAMPLES];
	  emitter->sample(info.p, samples, SHADOW_SAMPLES); 

	  color_t light;
	  for (auto& sample : samples) {
            auto in  = sample.p - info.p;
	    in.normalize();

	    if (dot(in, info.n) > 0) {
	      shading_info_t shadow;
	      if (scene.intersect(ray_t(info.p + info.n * 0.0001, in), shadow)) {
		if (static_cast<const thing_t*>(emitter->thing.get()) ==
		    static_cast<const thing_t*>(shadow.thing)) {
		  auto s  = 1.0/(sample.pdf*shadow.d*shadow.d);
		  auto il = info.b.to_local(in);
		  auto ol = info.b.to_local(out);
		  light += (emitter->emit() * bxdf->f(il, ol)).scale(s);
		}
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

  color_t indirect(const shading_info_t& info) const {
    return {0, 0, 0};
  }
};
