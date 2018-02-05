#include "glass.hpp"

bxdf_t::p glass_t::at() const {
  auto brdf = dynamic_cast<brdf_t*>(r);
  brdf->k = k;
  auto btdf = dynamic_cast<btdf_t*>(t);
  btdf->k = k;
  btdf->etaA = etaA;
  btdf->etaB = etaB;
  return bsdf;
}
