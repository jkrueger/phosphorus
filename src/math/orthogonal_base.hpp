#pragma once

#include "vector.hpp"

#include <cmath>

struct orthogonal_base {
  vector_t a, b, c;

  inline orthogonal_base()
  {}
  
  inline orthogonal_base(const vector_t& n)
    : a(cross(n, std::abs(n.z) < 0.5 ? vector_t(0.0, 0.0, 1.0) : vector_t(0.0, -1.0, 0.0))),
      b(n),
      c(cross(a, n))
  {
    a.normalize();
    c.normalize();
  }

  inline orthogonal_base(const vector_t& z, const vector_t& y)
    : a(cross(z,y)), b(y), c(z)
  {
    b.normalize();
  }

  inline vector_t to_world(const vector_t& v) const {
    return v.x * a + v.y * b + v.z * c;
  }
};

struct invertible_base : public orthogonal_base {
  vector_t ia, ib, ic;

  inline invertible_base()
  {}

  inline invertible_base(const vector_t& n)
    : orthogonal_base(n),
      // transpose base vectors
      ia(a.x,b.x,c.x),
      ib(a.y,b.y,c.y),
      ic(a.z,b.z,c.z)
  {}

  inline vector_t to_local(const vector_t& v) const {
    return v.x * ia + v.y * ib + v.z * ic;
  }
};
