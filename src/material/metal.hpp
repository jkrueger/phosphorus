#pragma once

#include "material.hpp"
#include "precision.hpp"

struct metal_t : public material_t {
  color_t k;
  float_t eta;
  float_t rough;

  metal_t(const color_t& k)
    : k(k)
  {}

  bxdf_t::p at(allocator_t& a) const;
};
