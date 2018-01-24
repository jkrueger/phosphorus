#pragma once

#include "precision.hpp"
#include "math/ray.hpp"
#include "math/sampling.hpp"
#include "shading.hpp"
#include "util/color.hpp"

thread_local std::random_device rd;
thread_local std::mt19937 gen(rd());
thread_local std::uniform_real_distribution<float_t> dis(0.0f,1.0f);

struct single_path_t {
  const uint32_t spd;
  const uint32_t samples;
  const uint8_t  max_depth;

  inline single_path_t(uint32_t spd, uint32_t max_depth)
    : spd(spd), samples(spd*spd), max_depth(max_depth)
  {}

  template<typename Scene>
  color_t trace(const Scene& scene, const ray_t& ray) const {
    color_t out;
    color_t beta(1.0);
    ray_t   path = ray;

    uint8_t depth = 0;
    while (depth <= max_depth) {

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

      // sample the bsdf based on the previous path direction transformed
      // into tangent space of the hit point
      const auto pl  = info.b.to_local(-path.direction);
      const auto r   = info.bxdf()->sample(pl, sample, next);

      // transform the sampled direction back to world
      path.direction = info.b.to_world(next.sampled);
      path.origin    = info.p + info.n * 0.0001;

      beta = beta * (r * (std::abs(dot(path.direction, info.n)) / next.pdf));

      if (depth > 3) {
	float_t q = std::max((float_t) 0.05f, 1.0f - beta.y());
	if (dis(gen) < q) {
	  break;
	}
	beta *= (1.0f / (1.0f - q));
      }

      ++depth;
    }

    return out;
  }

  template<typename Scene>
  color_t direct(const Scene& scene, shading_info_t& info, const ray_t& ray) const {
    color_t r;

    const auto& bxdf = info.bxdf();

    if (scene.lights.size() > 0 && bxdf->has_distribution()) {
      auto wi  = ray.origin - info.p;
      wi.normalize();

      sample_t         uv[samples];
      sampled_vector_t light_samples[samples];

      for (auto& emitter : scene.lights) {
	sampling::strategies::stratified_2d(uv, spd);
	emitter->sample(info.p, uv, light_samples, samples); 

	color_t light;
	for (const auto& sample : light_samples) {
	  auto wo  = sample.sampled - info.p;
	  auto d   = wo.length() - 0.0002;
	  wo.normalize();

	  if (dot(wo, info.n) > 0) {
	    if (!scene.occluded(ray_t(info.p + info.n * 0.0001, wo), d)) {
	      auto wil = info.b.to_local(wo);
	      auto wol = info.b.to_local(wi);
	      auto s   = wil.y/(sample.pdf*d*d);
	      light += (emitter->emit() * bxdf->f(wil, wol)).scale(s);
	    }
	  }
	}
	r += light.scale(1.0/samples);
      }
    }
    r.scale(1.0/scene.lights.size());
    return r;
  }
};
