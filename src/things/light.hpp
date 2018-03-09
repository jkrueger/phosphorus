#pragma once

#include "precision.hpp"
#include "thing.hpp"
#include "math/orthogonal_base.hpp"
#include "util/color.hpp"

struct light_t {
  typedef light_t* p;

  virtual ~light_t()
  {}

  virtual void sample(
    const vector_t& p,
    const sample_t* samples,
    sampled_vector_t* out, uint32_t n) const = 0;

  virtual color_t emit() const = 0;

  virtual color_t power() const  = 0;
};
