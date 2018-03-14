#pragma once

#include "precision.hpp"
#include "thing.hpp"
#include "math/orthogonal_base.hpp"
#include "util/color.hpp"

namespace light {
  struct area_t : public light_t {
    const surface_t::p surface;
    const float_t      area;
    color_t      emissive;
    vector_t     position;

    area_t(const vector_t& p, const surface_t::p& s, const color_t& e)
      : surface(s),
	area(0.0),
	emissive(e),
	position(p)
    {}

    void sample(
      const segment_t& s
    , const sample_t* samples
    , sampled_vector_t* out
    , uint32_t n) const
    {
      surface->sample(s.p, samples, out, n);

      orthogonal_base_t base((s.p - position).normalize());

      for (auto i=0; i<n; ++i) {
	out[i].sampled = base.to_world(out[i].sampled) + position;
      }
    }

    color_t emit(const vector_t&, vector_t&) const {
      return emissive;
    }

    color_t power() const {
      return emissive * area * M_PI;
    }
  };
}
