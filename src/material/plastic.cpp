#include "plastic.hpp"
#include "shading.hpp"

bxdf_t::p plastic_t::at(const shading_info_t&) const {
  std::dynamic_pointer_cast<bxdf::lambert_t>(diffuse)->k = kd;
  std::dynamic_pointer_cast<bxdf::simple_specular_t>(specular)->set(ks, s);
  return bsdf;
}
