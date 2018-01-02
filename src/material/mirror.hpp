#pragma once

#include "material.hpp"
#include "bxdf/reflection.hpp"

struct mirror_t : public material_t {
  typedef bxdf::specular_reflection_t<fresnel::none_t> perfect_reflector_t;
  
  color_t   k;
  bxdf_t::p r;

  mirror_t(const color_t& _k)
    : k(_k), r(new perfect_reflector_t())
  {}

  bxdf_t::p at(const shading_info_t& info) const;
};
