#include "bvh.hpp"

struct bvh_t::node_t {

  node_t* a, b;
  
  uint32_t offset; // offset into the things vector
  uint32_t num;    // number of things in this leaf
};

struct build_info_t {
  uint32_t index;
  aabb_t   bounds;
  vector_t centroid;

  build_info_t(uint32_t i, const aabb_t& b)
    : index(i), bounds(b), centroid(b.centroid())
  {}
};

bvh_t::node_t* build(uint32_t start, uint32_t& end, std::vector& infos) {
  bvh_t::node_t* out;

  auto n = (end - start);
  if (n == 1) {
    // make leaf
  }
  else {
    // make interior node
  }
}

bvh_t::bvh_t() {
}

void bvh_t::add(const thing_t::p& thing) {
  things.push_back(thing);
}

void bvh_t::build() {
  std::vector<build_info_t> infos(things.size());

  for (auto i=0; i<things.size(); ++i) {
    infos.push_back({i, thing->bounds()});
  }

  
}

bool bvh_t::intersect(const ray_t& ray, shading_info_t& info) {
  
}
