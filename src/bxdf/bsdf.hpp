#pragma once

#include "precision.hpp"
#include "shading.hpp"

#include <algorithm>

struct bsdf_t : public bxdf_t {
  typedef bsdf_t* p;
  
  static const uint8_t MAX_BXDF = 8;

  bsdf_t()
    : bxdf_t(bxdf_t::NONE), num_bxdf(0)
  {}

  void add(const bxdf_t::p& bxdf) {
    if (num_bxdf < MAX_BXDF) {
      bxdfs[num_bxdf++] = bxdf;
      flags = flags | bxdf->flags;
    }
  }

  color_t f(const vector_t& in, const vector_t& out) const {
    color_t c;
    for (auto i=0; i<num_bxdf; ++i) {
      c += bxdfs[i]->f(in, out);
    }
    return c;
  }

  color_t sample(const vector_t& v, const sample_t& sample, sampled_vector_t& out) const {
    auto index = std::min((int32_t) std::floor(sample.u * num_bxdf), (num_bxdf-1));
    auto f     = bxdfs[index]->sample(v, sample, out);

    for (auto i=0; i<num_bxdf; ++i) {
      bool reflect = v.y * out.sampled.y > 0;
      if (i != index &&
	  ((reflect && bxdfs[i]->is(REFLECTIVE)) ||
	   (!reflect && bxdfs[i]->is(TRANSMISSIVE)))) {
	f += bxdfs[i]->f(out.sampled, v);
	out.pdf += bxdfs[i]->pdf(out.sampled, v);
      }
    }

    out.pdf /= num_bxdf;
    
    return f;
  }

  // avoid dynamic memory allocation, so we can optimize bxdf
  // allocation later on
  uint8_t   num_bxdf;
  bxdf_t::p bxdfs[MAX_BXDF];
};
