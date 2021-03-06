#include "plane.hpp"
#include "precision.hpp"
#include "shading.hpp"

#include <limits>

plane_t::plane_t(const vector_t& o, const vector_t& _n, const material_t::p& m)
  : shadable_t(o, m), n(_n)
{}

bool plane_t::intersect(const ray_t& ray, shading_info_t& info) const {
  auto a = dot(ray.direction, n);
  if (std::abs(a) > std::numeric_limits<float_t>::epsilon()) {
    auto x = position - ray.origin;
    auto d = dot(x, n) / a;

    if (d > 0) {
      return info.update(ray, d, this);
    }
  }
  return false;
}

void plane_t::shading_parameters(shading_info_t& info, const vector_t& p) const {
  info.n = n;
}

void plane_t::sample(
  const vector_t&,
  const sample_t*,
  sampled_vector_t* out,
  uint32_t) const {

  throw std::runtime_error("Uniformly sampling isn't implemnented for plane_t");
}

aabb_t plane_t::bounds() const {
  return aabb_t();
}
