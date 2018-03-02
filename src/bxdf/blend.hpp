#pragma once

#include "bxdf.hpp"

template<typename Blend, typename A, typename B>
struct blend_t : bxdf_t {
  const Blend blend;
  const A*    a;
  const B*    b;

  template<typename... TS>
  blend_t(const A* a, const B* b, const TS& ...args)
    : bxdf_t((flags_t)(a->flags | b->flags))
    , blend(args...)
    , a(a)
    , b(b)
  {}

  color_t f(const vector_t& in, const vector_t& out) const {
    auto s = blend(in.y);
    color_t ret;
    if (a->has_distribution()) {
      ret += s * a->f(in, out);
    }
    if (b->has_distribution()) {
      ret += (1.0f - s) * b->f(in, out);
    }
    return ret;
  }

  color_t sample(const vector_t& v, const sample_t& sample, sampled_vector_t& out) const {
    auto s = blend(v.y);
    if (sample.u < s) {
      const auto r = s * a->sample(v, sample, out);
      out.pdf = s;
      return r;
    }
    else {
       s = 1.0f - s;
       const auto r = s * b->sample(v, sample, out);
       out.pdf = s;
       return r;
    }
  }

  float_t pdf(const vector_t& in, const vector_t& out) const {
    return a->pdf(in, out) + b->pdf(in, out);
  }
};
