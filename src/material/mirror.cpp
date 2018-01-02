#include "mirror.hpp"

bxdf_t::p mirror_t::at(const shading_info_t& info) const {
  std::dynamic_pointer_cast<perfect_reflector_t>(r)->k = k;
  return r;
}
