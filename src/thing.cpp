#include "thing.hpp"
#include "shading.hpp"

bool thing_t::occluded(const ray_t& ray, float_t d) const {
  shading_info_t info;
  info.d = d;
  return intersect(ray, info);
}

