#pragma once

#include "shading.hpp"

#include <algorithm>

struct bsdf_t : public bxdf_t {
  typedef std::shared_ptr<bsdf_t> p;
  
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

  color_t sample(const vector_t& v, sample_t& sample) const {
    auto index = std::min((uint8_t) std::floor(sample.u * num_bxdf), num_bxdf);
    auto f     = bxdfs[index]->sample(v, sample);

    for (auto i=0; i<num_bxdf; ++i) {
      bool reflect = v.y * sample.p.y > 0;
      if (i != index &&
	  ((reflect && bxdfs[i]->is(REFLECTIVE)) ||
	   (!reflect && bxdfs[i]->is(TRANSMISSIVE)))) {
	f += bxdfs[i]->f(sample.p, v);
	sample.pdf += bxdfs[i]->pdf(sample.p, v);
      }
    }

    sample.pdf /= num_bxdf;
    
    return f;
  }

  // avoid dynamic memory allocation, so we can optimize bxdf
  // allocation later on
  uint8_t   num_bxdf;
  bxdf_t::p bxdfs[MAX_BXDF];
};
