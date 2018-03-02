#include "paint.hpp"

#include "bxdf/blend.hpp"
#include "bxdf/reflection.hpp"
#include "bxdf/lambert.hpp"
#include "bxdf/microfacet.hpp"

typedef bxdf::specular_reflection_t gloss_t;

 typedef bxdf::microfacet_t<
   microfacet::distribution::ggx_t
 , microfacet::shadowing::schlick_t
 , fresnel::dielectric_t
 > coat_t;

//typedef bxdf::lambert_t coat_t;

bxdf_t::p paint_t::at(allocator_t& a) const {
  auto gloss = new(a) gloss_t({.2,.2,.2});
  auto paint = new(a) coat_t(k, microfacet::distribution::ggx_t::roughness_to_alpha(.9f));
  //auto paint = new(a) coat_t(k);

  //return paint;
  //return gloss;
  return new(a) blend_t<fresnel::dielectric_t, gloss_t, coat_t>(gloss, paint, etaB, etaA);
}
