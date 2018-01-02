#pragma once

#include "material.hpp"
#include "bxdf/bsdf.hpp"
#include "bxdf/reflection.hpp"

struct glass_t : public material_t {
  typedef bxdf::specular_reflection_t<fresnel::dielectric_t>   brdf_t;
  typedef bxdf::specular_transmission_t<fresnel::dielectric_t> btdf_t;

  color_t k;
  double  etaA, etaB;
  bxdf_t::p r, t;
  bsdf_t::p bsdf;

  glass_t(const color_t& _k)
    : k(_k), etaA(1.0), etaB(1.5), r(new brdf_t(etaA, etaB)), t(new btdf_t(etaA, etaB)),
      bsdf(new bsdf_t())
  {
    bsdf->add(r);
    bsdf->add(t);
  }

  bxdf_t::p at(const shading_info_t& info) const;
};
