#include "diffuse.hpp"
#include "shading.hpp"

bxdf_t::p diffuse_reflector_t::at(const shading_info_t&) const {
  std::dynamic_pointer_cast<bxdf::lambert_t>(lambert)->k = k;
  return lambert;
}
