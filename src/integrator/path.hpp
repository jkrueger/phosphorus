#pragma once

#include "math/ray.hpp"
#include "math/sampling.hpp"
#include "shading.hpp"
#include "thing.hpp"
#include "things/light.hpp"
#include "util/color.hpp"

#include "bxdf/lambert.hpp"

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<> dis(0.0,1.0);

struct path_tracer_t {

  static const uint32_t SHADOW_SAMPLES = 9;
  static const uint8_t  MAX_DEPTH = 8;

  std::vector<light_t::p> emitters;

  color_t trace(const thing_t& scene, const ray_t& ray) const {
    color_t out;
    ray_t   path = ray;
    color_t beta(1.0);

    uint8_t depth = 0;
    while (depth <= MAX_DEPTH) {

      shading_info_t info;
      if (scene.intersect(path, info)) {
	out += beta * direct(scene, info, path);
	if (depth == 0) {
	  out += info.emissive;
	}
      }
      else {
	// path escaped the scene
	break;
      }

      sample_t sample(dis(gen), dis(gen));
      sampled_vector_t next;

      auto foo = info.b.to_local(-path.direction);
      auto r = info.bxdf()->sample(foo, sample, next);

      path = ray_t(info.p + info.n * 0.0001, info.b.to_world(next.sampled));
      beta = beta * (r * (std::abs(dot(path.direction, info.n)) / next.pdf));

      if (depth > 3) {
	double q = std::max((double) 0.05, 1.0 - beta.y());
	if (dis(gen) < q) {
	  break;
	}
	beta *= (1.0 / (1.0 - q));
      }

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

      sample_t         uv[SHADOW_SAMPLES];
      sampled_vector_t light_samples[SHADOW_SAMPLES];

      for (auto& emitter : emitters) {
	if (static_cast<const thing_t*>(emitter.get()) !=
	    static_cast<const thing_t*>(info.thing)) {
	  
	  sampling::strategies::stratified_2d(uv, 3);
	  emitter->sample(info.p, uv, light_samples, SHADOW_SAMPLES); 

	  color_t light;
	  for (auto& sample : light_samples) {
            auto in  = sample.sampled - info.p;
	    in.normalize();

	    if (dot(in, info.n) > 0) {
	      shading_info_t shadow;
	      if (scene.intersect(ray_t(info.p + info.n * 0.0001, in), shadow)) {
		if (static_cast<const thing_t*>(emitter->thing.get()) ==
		    static_cast<const thing_t*>(shadow.thing)) {
		  auto il = info.b.to_local(in);
		  auto ol = info.b.to_local(out);
		  auto s  = il.y/(sample.pdf*shadow.d*shadow.d);
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
};
