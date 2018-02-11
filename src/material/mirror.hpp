#pragma once

#include "material.hpp"
#include "bxdf/reflection.hpp"

struct mirror_t : public material_t {
  typedef bxdf::specular_reflection_t perfect_reflector_t;
  
  color_t k;

  mirror_t(const color_t& k)
    : k(k)
  {}

  bxdf_t::p at(allocator_t& a) const;
};
