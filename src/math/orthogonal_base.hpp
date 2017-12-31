#pragma once

#include "vector.hpp"

struct orthogonal_base {
  vector_t a, b, c;

  inline orthogonal_base(vector_t& n)
    : a(cross(n, std::abs(n.z) < 0.5 ? vector_t(0.0, 0.0, 1.0) : vector_t(0.0, -1.0, 0.0))),
      b(n),
      c(cross(a, n))
  {
    a.normalize();
    c.normalize();
  }

  inline vector_t to(const vector_t& v) const {
    return v.x * a + v.y * b + v.z * c;
  }
};
