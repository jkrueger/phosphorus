#include "plastic.hpp"
#include "shading.hpp"

bxdf_t::p plastic_t::at(const shading_info_t&) const {
  dynamic_cast<bxdf::lambert_t*>(diffuse)->k = kd;
  dynamic_cast<bxdf::simple_specular_t*>(specular)->set(ks, s);
  return bsdf;
}
