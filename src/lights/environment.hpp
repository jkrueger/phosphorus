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

    environment_t(const texture_t<color_t>::p& map)
      : map(map)
    {}

    void sample(
      const vector_t& p
    , const sample_t* samples
    , sampled_vector_t* out
    , uint32_t n) const
    {
      // sample sphere
    }

    color_t le(const segment_t& segment) const {
      return map->eval(segment.wi);
    }

    color_t emit() const {
      return 0;
    }

    color_t power() const {
      return 0;
    }
  };
}
