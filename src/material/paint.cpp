#include "paint.hpp"

#include "bxdf/blend.hpp"
#include "bxdf/reflection.hpp"
#include "bxdf/lambert.hpp"

typedef bxdf::specular_reflection_t gloss_t;
typedef bxdf::lambert_t             diffuse_t;

bxdf_t::p paint_t::at(allocator_t& a) const {
  auto gloss = new(a) gloss_t(k);
  auto paint = new(a) diffuse_t(k);

  return new(a) blend_t<fresnel::dielectric_t, gloss_t, diffuse_t>(gloss, paint, etaA, etaB);
}
