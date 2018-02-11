#include "diffuse.hpp"
#include "shading.hpp"

#include "bxdf/oren_nayar.hpp"

bxdf_t::p diffuse_reflector_t::at(allocator_t& a) const {
  return new(a) bxdf::oren_nayar_t(k, s);
}
