#include "plastic.hpp"
#include "shading.hpp"
#include "bxdf/lambert.hpp"
#include "bxdf/microfacet.hpp"
#include "bxdf/bsdf.hpp"

typedef bxdf::lambert_t diffuse_t;

typedef bxdf::microfacet_t<
  microfacet::distribution::ggx_t
, microfacet::shadowing::schlick_t
, fresnel::dielectric_t
> gloss_t;

bxdf_t::p plastic_t::at(allocator_t& a) const {
  bsdf_t::p bsdf = new(a) bsdf_t();
  bsdf->add(new(a) diffuse_t(kd));
  bsdf->add(new(a) gloss_t(ks, microfacet::distribution::ggx_t::roughness_to_alpha(roughness)));

  return bsdf;
}
