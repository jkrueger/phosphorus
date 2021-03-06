#pragma once

#include "bxdf.hpp"
#include "precision.hpp"
#include "math/ray.hpp"
#include "math/sampling.hpp"
#include "math/vector.hpp"
#include "shading.hpp"
#include "things/scene.hpp"
#include "util/color.hpp"

#include "bxdf/reflection.hpp"

thread_local std::mt19937 gen;
thread_local std::uniform_real_distribution<float_t> dis(0.0f,1.0f);

struct single_path_t {
  const uint8_t max_depth;

  occlusion_query_t* shadows;
  invertible_base_t* tagent_spaces;

  inline single_path_t(uint32_t max_depth)
    : max_depth(max_depth)
  {}

  inline void allocate(allocator_t& a, uint32_t n) {
    shadows       = new(a) occlusion_query_t[n];
    tagent_spaces = new(a) invertible_base_t[n];
  }

  template<typename Scene>
  inline void sample_lights(
    const Scene& scene
  , const segment_t* stream
  , const active_t& active)
  {
    auto ts = tagent_spaces;

    for (auto i=0; i<active.num; ++i) {
      auto index = active.segment[i];
      const auto& segment = stream[index];
      if (segment.is_hit()) {
	auto& shadow = shadows[index];

	auto  l     = (size_t) (dis(gen) * scene.lights.size());
	auto& light = scene.lights[std::min(l, scene.lights.size()-1)];

	sample_t uv = { dis(gen), dis(gen) };
	sampled_vector_t sample;

	light->sample(segment, &uv, &sample, 1);

	shadow.p  = segment.p;
	shadow.wi = sample.sampled - segment.p;
	if (in_same_hemisphere(shadow.wi, segment.n)) {
	  shading::offset(shadow, segment.n);
	  shadow.d     = shadow.wi.length();
	  shadow.wi.normalize();
	  shadow.pdf   = sample.pdf;
	  shadow.e     = light->emit(shadow.p, shadow.wi);
	  shadow.flags = 0;
	}
	else {
	  shadow.mask();
	}

	new(ts+index) invertible_base_t(segment.n);
      }
    }

    scene.occluded(shadows, active);
  }

  template<typename Scene, typename Splat>
  inline void shade(
    const Scene& scene
  , const bxdf_t::p bxdf
  , const segment_t* stream
  , const active_t& active
  , Splat& splats)
  {
    for (auto i=0; i<active.num; ++i) {
      const auto  index   = active.segment[i]; 
      const auto& shadow  = shadows[index];
      const auto& segment = stream[index];
      const auto& tagent_space = tagent_spaces[index];

      const auto il = tagent_space.to_local(shadow.wi);
      const auto ol = tagent_space.to_local(-segment.wi);

      color_t r;
      float_t s = 1;

      if (segment.is_hit()) {
	if (!shadow.occluded()) {
	  s = il.y / shadow.pdf;
	  r = segment.beta * (shadow.e * bxdf->f(il, ol)).scale(s);
	}
      }
      else {
	if (segment.depth == 0 || bxdf->is_specular()) {
	  //s = dot(segment.wi, segment.n);
	  r = segment.beta * scene.le(segment.wi);
	  //std::cout << s << std::endl;
	}
      }

      splats[index].c += r;
    }
  }

  inline void sample_path_directions(
    const bxdf_t::p bxdf
  , segment_t* stream
  , const active_t& active
  , active_t& out)
  {
    for (auto i=0; i<active.num; ++i) {
      auto  index   = active.segment[i];
      auto& segment = stream[index];

      if (segment.is_hit()) {
      const auto& tagent_space = tagent_spaces[index];

      sample_t uv = { dis(gen), dis(gen) };
      sampled_vector_t next;

      // sample the bsdf based on the previous path direction transformed
      // into the tangent space of the hit point
      const auto pl = tagent_space.to_local(-segment.wi);
      const auto f  = bxdf->sample(pl, uv, next);

      // transform the sampled direction back to world
      segment.wo   = segment.wi;
      segment.wi   = tagent_space.to_world(next.sampled);
      segment.beta = segment.beta * (f * (abs_dot(segment.wi, segment.n) / next.pdf));
      
      shading::offset(segment, segment.n);

      if (segment.depth < max_depth &&
	  (segment.depth < 3 || !terminate_ray(segment.beta))) {
	out.segment[out.num++] = active.segment[i];
      }
      else {
	segment.kill();
      }

      ++segment.depth;
      }
      else {
	segment.kill();
      }
    }
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
