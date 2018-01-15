#pragma once

#include "material.hpp"
#include "precision.hpp"
#include "bxdf/lambert.hpp"
#include "bxdf/specular.hpp"
#include "bxdf/bsdf.hpp"

struct plastic_t : public material_t {
  color_t   kd, ks;
  float_t    s;
  bxdf_t::p diffuse, specular;
  bsdf_t::p bsdf;

  plastic_t(const color_t& _kd, const color_t& _ks, float_t _s)
    : kd(_kd), ks(_ks), s(_s),
      diffuse(new bxdf::lambert_t()),
      specular(new bxdf::simple_specular_t()),
      bsdf(new bsdf_t())
  {
    bsdf->add(diffuse);
    bsdf->add(specular);
  }

  bxdf_t::p at(const shading_info_t&) const;
};
