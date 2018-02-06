#pragma once

#include "direct.hpp"
#include "precision.hpp"
#include "math/ray.hpp"
#include "math/sampling.hpp"
#include "math/vector.hpp"
#include "shading.hpp"
#include "things/scene.hpp"
#include "util/color.hpp"

thread_local std::mt19937 gen;
thread_local std::uniform_real_distribution<float_t> dis(0.0f,1.0f);

struct single_path_t {
  const uint8_t  max_depth;
  const direct_t direct;

  inline single_path_t(uint32_t spd, uint32_t max_depth)
    : max_depth(max_depth)
    , direct(1)
  {}

  template<typename Scene>
  color_t li(const Scene& scene, const vector_t& wo, const shading_info_t& i) const {
    color_t out;
    color_t beta(1.0);
    ray_t   path = {i.p, wo};

    uint8_t depth = 0;
    shading_info_t info = i;
    while (depth <= max_depth) {
      sample_t sample(dis(gen), dis(gen));
      sampled_vector_t next;

      const auto bxdf = info.bxdf();

      // sample the bsdf based on the previous path direction transformed
      // into tangent space of the hit point
      const auto pl = info.b.to_local(-path.direction);
      const auto r  = bxdf->sample(pl, sample, next);

      // transform the sampled direction back to world
      path = info.ray(info.b.to_world(next.sampled));
      beta = beta * (r * (abs_dot(path.direction, info.n) / next.pdf));

      if (depth > 2 && terminate_ray(beta)) {
      	break;
      }

      if (scene.intersect(path, info)) {
	auto  l     = (int) (dis(gen) * scene.lights.size());
	auto& light = scene.lights[l];

	sample_t uv = { dis(gen), dis(gen) };
	sampled_vector_t sample;
	light->sample(info.p, &uv, &sample, 1);

	auto wo = -path.direction;
	auto wi = sample.sampled - info.p;
	const auto d = wi.length();

	wi.normalize();

	if (in_same_hemisphere(wi, info.n) && !scene.occluded(info.ray(wi), d)) {
	  const auto il = info.b.to_local(wi);
	  const auto ol = info.b.to_local(wo);
	  const auto s  = il.y/(sample.pdf*d*d);

	  out += beta * (light->emit() * bxdf->f(il, ol)).scale(s);
	}
      }
      else {
	// path escaped the scene
	break;
      }
	
      ++depth;
    }

    return out;
  }

  inline bool terminate_ray(color_t& beta) const {
    float_t q = std::max((float_t) 0.05f, 1.0f - beta.y());
    if (dis(gen) < q) {
      return true;
    }
    beta *= (1.0f / (1.0f - q));
    return false;
  }
};
