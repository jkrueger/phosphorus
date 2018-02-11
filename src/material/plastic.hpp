#pragma once

#include "material.hpp"
#include "precision.hpp"

struct plastic_t : public material_t {
  color_t   kd, ks;
  float_t   s;

  plastic_t(const color_t& _kd, const color_t& _ks, float_t _s)
    : kd(_kd), ks(_ks), s(_s)
  {
  }

  bxdf_t::p at(allocator_t& a) const;
};
