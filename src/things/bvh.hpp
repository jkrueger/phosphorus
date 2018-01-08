#pragma once

#include "thing.hpp"

#include <memory>
#include <vector>

struct bvh_t : public thing_t {

  struct node_t;

  void add(const thing_t::p& thing);

  void build();
  
  bool intersect(const ray_t&, shading_info_t&) const;

  std::vector<thing_t::p> things;
  std::shared_ptr<node_t> root;
};
