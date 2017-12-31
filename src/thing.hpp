#pragma once

#include "math/sampling.hpp"
#include "math/vector.hpp"
#include "material.hpp"

#include <memory>
#include <vector>

struct ray_t;
struct shading_info_t;

struct thing_t {
  typedef std::shared_ptr<thing_t> p;

  vector_t position;

  inline thing_t()
  {}

  inline thing_t(const vector_t& p)
    : position(p)
  {}

  virtual bool intersect(const ray_t&, shading_info_t&) const = 0;

  virtual void sample(const vector_t& p, sample_t*, uint32_t) const = 0;
};

struct shadable_t : public thing_t {

  material_t::p material;

  inline shadable_t(const vector_t& p, const material_t::p& m)
    : thing_t(p), material(m)
  {}

  virtual void shading_parameters(shading_info_t& info, const vector_t& p) const = 0;
};

struct things_t : public thing_t {
  
  std::vector<std::shared_ptr<thing_t>> things;

  bool intersect(const ray_t& ray, shading_info_t& info) const {
    bool hit_anything = false;
    for (auto thing : things) {
      bool hit_thing = thing->intersect(ray, info);
      hit_anything |= hit_thing;
    }
    return hit_anything;
  }

  void sample(const vector_t&, sample_t*, uint32_t) const {
    throw std::runtime_error("Uniform sampling not implemented for things_t");
  }
};
