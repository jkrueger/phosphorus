#pragma once

#include "mesh.hpp"

#include "math/parametric/square.hpp"
#include "math/vector.hpp"

namespace tesselate {
  template<typename T>
  inline mesh_t::p surface(const T& surface, const material_t::p& m) {
  }

  template<>
  inline mesh_t::p surface<parametric::rectangle_t>(
    const parametric::rectangle_t& rect,
    const material_t::p& m) {

    float hw = rect.width * 0.5f;
    float hh = rect.height * 0.5f;

    vector_t v[] = {
      {-hw, .3298f, hh },
      { hw, .3298f, hh },
      { hw, .3298f, -hh },
      {-hw, .3298f, -hh }
    };

    vector_t n[] = {
      {0, 1, 0},
      {0, 1, 0},
      {0, 1, 0},
      {0, 1, 0}
    };

    uint32_t f[] = {
      0, 1, 2,
      0, 2, 3
    };

    return mesh_t::p(new mesh_t(m, v, n, f, 4, 6));
  }
}
