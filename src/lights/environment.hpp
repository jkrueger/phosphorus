#pragma once

#include "things/light.hpp"
#include "precision.hpp"
#include "shading.hpp"
#include "thing.hpp"
#include "texture.hpp"
#include "util/color.hpp"

namespace light {
  struct environment_t : public light_t {
    typedef environment_t* p;
    
    const texture_t<color_t>::p map;
    const float_t radius;

    environment_t(const texture_t<color_t>::p& map)
      : map(map)
      , radius(1000.0f)
    {}

    void sample(
      const segment_t& s
    , const sample_t* samples
    , sampled_vector_t* out
    , uint32_t n) const
    {
      orthogonal_base_t base(s.n);

      for (auto i=0; i<n; ++i) {
	sampling::hemisphere::uniform(samples[i], out[i]);
	out[i].sampled = base.to_world(out[i].sampled.scale(radius*2));
      }
    }

    color_t le(const vector_t& wi) const {
      return map->eval(wi);
    }

    color_t emit(const vector_t& p, vector_t& wi) const {
      return le(wi);
    }

    color_t power() const {
      return 0;
    }
  };
}
