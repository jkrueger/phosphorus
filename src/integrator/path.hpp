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

  template<typename Scene>
  inline color_t li(const Scene& scene, segment_t& segment, const bxdf_t::p bxdf) const {
    color_t r;

    auto  l     = (int) (dis(gen) * scene.lights.size());
    auto& light = scene.lights[l];

    sample_t uv = { dis(gen), dis(gen) };
    sampled_vector_t sample;

    light->sample(segment.p, &uv, &sample, 1);

    segment.wo = segment.wi;
    segment.wi = sample.sampled - segment.p;

    const auto d = segment.wi.length();

    segment.wi.normalize();

    const invertible_base tagent_space(segment.n);

    if (in_same_hemisphere(segment.wi, segment.n) && !scene.occluded(segment, d)) {
      const auto il = tagent_space.to_local(segment.wi);
      const auto ol = tagent_space.to_local(segment.wo);
      const auto s  = il.y/(sample.pdf*d*d);

      r += segment.beta * (light->emit() * bxdf->f(il, ol)).scale(s);
    }

    uv = { dis(gen), dis(gen) };
    sampled_vector_t next;

    // sample the bsdf based on the previous path direction transformed
    // into the tangent space of the hit point
    const auto pl = tagent_space.to_local(-segment.wo);
    const auto f  = bxdf->sample(pl, uv, next);

    // transform the sampled direction back to world
    segment.wi   = tagent_space.to_world(next.sampled);
    segment.beta = segment.beta * (f * (abs_dot(segment.wi, segment.n) / next.pdf));

    if (segment.depth >= max_depth || (segment.depth > 2 && terminate_ray(segment.beta))) {
      segment.kill();
    }

    ++segment.depth;

    return r;
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
