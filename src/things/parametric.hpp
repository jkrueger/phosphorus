#pragma once

#include "thing.hpp"
#include "math/parametric/sphere.hpp"
#include "math/parametric/square.hpp"

namespace things {
  template<typename T>
  struct parametric_t : public surface_t //, thing_t
  {
    T parametric;

    template<typename... Args>
    inline parametric_t(const Args& ...args)
      : parametric(args...)
    {};
    
    void sample(
      const vector_t& p,
      const sample_t* samples,
      sampled_vector_t* out, uint32_t num) const {
      
      parametric.sample(p, samples, out, num);
    }

    void tesselate(const std::vector<triangle_t::p>& triangles);
  };

  typedef parametric_t<parametric::sphere_t> sphere_t;
  typedef parametric_t<parametric::rectangle_t> rectangle_t;
}
