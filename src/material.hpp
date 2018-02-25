#pragma once

#include "bxdf.hpp"
#include "util/allocator.hpp"

#include <memory>

struct bxdf_t;
struct shading_info_t;

struct material_t {
  typedef material_t* p;

  uint32_t id;

  material_t();

  virtual bxdf_t* at() const
  { return nullptr; }

  virtual bxdf_t* at(allocator_t& allocator) const
  { return nullptr; }
};
