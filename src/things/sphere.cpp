#include "sphere.hpp"
#include "shading.hpp"

#include "math/orthogonal_base.hpp"

#include <math.h>

sphere_t::sphere_t(const vector_t& c, double r, const material_t::p& m)
  : shadable_t(c, m), radius(r), radius2(r*r)
{}

bool sphere_t::intersect(const ray_t& ray, shading_info_t& info) const {
  auto l   = position - ray.origin;
  auto tca = dot(l, ray.direction);

  if (tca < 0) { return false; }
  
  auto d2  = dot(l, l) - (tca * tca);

  if (d2 > radius2) { return false; }

  auto thc = sqrt(radius2 - d2);
  auto t0  = tca - thc;
  auto t1  = tca + thc;

  if (t0 > t1) {
    std::swap(t0, t1);
  }

  if (t0 < 0) {
    t0 = t1;
    if (t0 < 0) {
      return false;
    }
  }

  return info.update(ray, t0, *this);
}

void sphere_t::shading_parameters(shading_info_t& info, const vector_t& p) const {
  info.n = (p - position);
  info.n.normalize();
}

void sphere_t::sample(const vector_t& p, sample_t* samples, uint32_t num) const {
  const orthogonal_base base((p - position).normalize());
  const auto pdf = 1.0 / (M_PI*radius*radius);

  for (auto i=0;i<num; ++i) {
    const float r   = sqrt(1.0 - samples[i].u * samples[i].u);
    const float phi = 2 * M_PI * samples[i].v;

    vector_t v(cos(phi) * r, samples[i].u, sin(phi) * r);

    samples[i].pdf = pdf;
    samples[i].p = base.to_world(v).scale(radius) + position;
  }
}
