#include "plastic.hpp"
#include "shading.hpp"
#include "bxdf/lambert.hpp"
#include "bxdf/specular.hpp"
#include "bxdf/bsdf.hpp"

bxdf_t::p plastic_t::at(allocator_t& a) const {
  bsdf_t::p bsdf = new(a) bsdf_t();
  bsdf->add(new(a) bxdf::lambert_t(kd));
  bsdf->add(new(a) bxdf::simple_specular_t(ks, s));
  
  return bsdf;
}
