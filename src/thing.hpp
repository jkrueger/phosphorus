#pragma once

#include "things/triangle.hpp"
#include "math/sampling.hpp"
#include "math/vector.hpp"

#include <memory>
#include <vector>

struct ray_t;

struct thing_t {
  typedef std::shared_ptr<thing_t> p;

  virtual void tesselate(std::vector<triangle_t::p>& triangles) const = 0;
};

struct surface_t {
  typedef std::shared_ptr<surface_t> p;

  virtual void sample(const vector_t& p, const sample_t*, sampled_vector_t*, uint32_t) const = 0;
};
