#pragma once

#include "material.hpp"
#include "precision.hpp"

struct glass_t : public material_t {
  color_t k;
  float_t etaA, etaB;

  glass_t(const color_t& k)
    : k(k), etaA(1.0), etaB(1.5)
  {}

  bxdf_t::p at(allocator_t& a) const;
};
