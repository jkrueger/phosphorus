#include "glass.hpp"

#include "bxdf/blend.hpp"
#include "bxdf/reflection.hpp"

typedef bxdf::specular_reflection_t   brdf_t;
typedef bxdf::specular_transmission_t btdf_t;

bxdf_t::p glass_t::at(allocator_t& a) const {
  auto brdf = new(a) brdf_t(k);
  auto btdf = new(a) btdf_t(k, etaA, etaB);

  return new(a) blend_t<fresnel::dielectric_t, brdf_t, btdf_t>(brdf, btdf, etaA, etaB);
}
