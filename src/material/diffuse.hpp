#pragma once

#include "material.hpp"
#include "util/color.hpp"

struct diffuse_reflector_t : public material_t {
  color_t k;
  float_t s;

  diffuse_reflector_t(const color_t& k, float_t s)
    : k(k), s(s)
  {}

  bxdf_t::p at(allocator_t& allocator) const;
};
