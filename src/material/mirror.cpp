#include "mirror.hpp"

bxdf_t::p mirror_t::at() const {
  dynamic_cast<perfect_reflector_t*>(r)->k = k;
  return r;
}
