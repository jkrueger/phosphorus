#pragma once

#include "material.hpp"
#include "precision.hpp"

struct glass_t : public material_t {
  color_t k;
  float_t etaA, etaB;

  glass_t(const color_t& k)
    : k(k), etaA(1), etaB(2.1)
  {}

  bxdf_t::p at(allocator_t& a) const;
};
