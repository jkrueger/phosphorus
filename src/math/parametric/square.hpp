#pragma once

namespace parametric {
  struct rectangle_t {
    float width, height;
    
    void sample(
      const vector_t& p,
      const sample_t* samples,
      sampled_vector_t* sampled,
      uint32_t num) const;
  };
}
