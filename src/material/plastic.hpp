#pragma once

#include "material.hpp"
#include "precision.hpp"

struct plastic_t : public material_t {
  color_t   kd, ks;
  float_t   roughness;

  plastic_t(const color_t& kd, const color_t& ks, float_t r)
    : kd(kd), ks(ks), roughness(r)
  {}

  bxdf_t::p at(allocator_t& a) const;
};
