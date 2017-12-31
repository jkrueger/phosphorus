#include "material.hpp"
#include "bxdf/lambert.hpp"

struct diffuse_reflector_t : public material_t {
  color_t   k;
  bxdf_t::p lambert;
  
  diffuse_reflector_t(const color_t& _k)
    : k(_k), lambert(new bxdf::lambert_t())
  {}

  bxdf_t::p at(const shading_info_t&) const;
};
