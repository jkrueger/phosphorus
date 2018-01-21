#pragma once

#include <memory>

struct bxdf_t;
struct shading_info_t;

struct material_t {
  typedef std::shared_ptr<material_t> p;

  virtual bxdf_t* at(const shading_info_t& info) const = 0;
};
