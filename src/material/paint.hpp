#pragma once

#include "material.hpp"
#include "precision.hpp"

struct paint_t : public material_t {
  color_t k;
  float_t etaA, etaB;

  paint_t(const color_t& k)
    : k(k), etaA(1.0), etaB(1.63)
  {}

  bxdf_t::p at(allocator_t& a) const;
};
