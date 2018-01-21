#include "diffuse.hpp"
#include "shading.hpp"

bxdf_t::p diffuse_reflector_t::at(const shading_info_t&) const {
  dynamic_cast<bxdf::lambert_t*>(lambert)->k = k;
  return lambert;
}
