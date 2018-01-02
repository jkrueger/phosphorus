#include "glass.hpp"

bxdf_t::p glass_t::at(const shading_info_t& info) const {
  auto brdf = std::dynamic_pointer_cast<brdf_t>(r);
  brdf->k = k;
  auto btdf = std::dynamic_pointer_cast<btdf_t>(t);
  btdf->k = k;
  btdf->etaA = etaA;
  btdf->etaB = etaB;
  return bsdf;
}
