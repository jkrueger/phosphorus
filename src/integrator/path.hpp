#pragma once

#include "bxdf.hpp"
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
  const uint8_t max_depth;

  inline single_path_t(uint32_t max_depth)
    : max_depth(max_depth)
  {}

  template<typename Scene, typename Splat>
  inline void li(const Scene& scene, segment_t* stream, active_t& active, active_t& out, Splat* splats, const bxdf_t::p bxdf) const {
    static thread_local occlusion_query_t shadows[4096];
    static thread_local invertible_base_t tagent_spaces[4096];

    auto ts = tagent_spaces;

    for (auto i=0; i<active.num; ++i) {
      auto& segment = stream[active.segment[i]];
      auto& shadow  = shadows[active.segment[i]];

      auto  l     = (int) (dis(gen) * scene.lights.size());
      auto& light = scene.lights[l];

      sample_t uv = { dis(gen), dis(gen) };
      sampled_vector_t sample;

      light->sample(segment.p, &uv, &sample, 1);

      shadow.p  = segment.p;
      shadow.wi = sample.sampled - segment.p;

      if (in_same_hemisphere(shadow.wi, segment.n)) {
        shading::offset(shadow, segment.n);
	shadow.d     = shadow.wi.length();
	shadow.wi.normalize();
	shadow.pdf   = sample.pdf;
	shadow.e     = light->emit();
	shadow.flags = 0;
      }
      else {
	shadow.mask();
      }

      new(ts++) invertible_base_t(segment.n);
    }

    scene.occluded(shadows, active);

    for (auto i=0; i<active.num; ++i) {
      auto  index   = active.segment[i]; 
      auto& shadow  = shadows[index];
      auto& segment = stream[index];
      const auto& tagent_space = tagent_spaces[i];

      if (!shadow.occluded()) {
	const auto il = tagent_space.to_local(shadow.wi);
	const auto ol = tagent_space.to_local(segment.wi);
	const auto s  = il.y/(shadow.pdf*shadow.d*shadow.d);

	splats[index].c += segment.beta * (shadow.e * bxdf->f(il, ol)).scale(s);
      }

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

      if (segment.depth < max_depth && (segment.depth < 3 || !terminate_ray(segment.beta))) {
	out.segment[out.num++] = active.segment[i];
      }

      ++segment.depth;
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
