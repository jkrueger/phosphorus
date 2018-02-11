#include "mirror.hpp"

bxdf_t::p mirror_t::at(allocator_t& a) const {
  return new(a) perfect_reflector_t(k);
}
