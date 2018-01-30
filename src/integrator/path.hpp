#pragma once

#include "direct.hpp"
#include "precision.hpp"
#include "math/ray.hpp"
#include "math/sampling.hpp"
#include "math/vector.hpp"
#include "shading.hpp"
#include "things/scene.hpp"
#include "util/color.hpp"

thread_local std::random_device rd;
thread_local std::mt19937 gen(rd());
thread_local std::uniform_real_distribution<float_t> dis(0.0f,1.0f);

struct single_path_t {
  const uint8_t  max_depth;
  const direct_t direct;

  inline single_path_t(uint32_t spd, uint32_t max_depth)
    : max_depth(max_depth)
    , direct(spd)
  {}

  template<typename Scene>
  color_t li(const Scene& scene, const ray_t& ray) const {
    color_t out;
    color_t beta(1.0);
    ray_t   path = ray;

    uint8_t depth = 0;
    while (depth <= max_depth) {

      shading_info_t info;
      if (scene.intersect(path, info)) {
	out += beta * direct.li(scene, (path.origin - info.p), info);
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
      const auto pl = info.b.to_local(-path.direction);
      const auto r  = info.bxdf()->sample(pl, sample, next);

      // transform the sampled direction back to world
      path = info.ray(info.b.to_world(next.sampled));
      beta = beta * (r * (abs_dot(path.direction, info.n) / next.pdf));

      if (depth > 3 && terminate_ray(beta)) {
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
