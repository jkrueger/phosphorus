#pragma once

#include "light.hpp"
#include "thing.hpp"
#include "util/stats.hpp"

#include <vector>

template<typename Accel>
struct scene_t {
  Accel accel;

  std::vector<light_t::p> lights;
  std::vector<thing_t::p> things;

  stats_t::p stats;

  scene_t(const stats_t::p& s)
    : stats(s)
  {}

  void preprocess();
  
  bool intersect(const ray_t& ray, shading_info_t& info) const;

  bool occluded(const ray_t& ray, float_t d) const;

  inline void add(const thing_t::p& thing) {
    things.push_back(thing);
  }

  inline void add(const light_t::p& light) {
    lights.push_back(light);
    // TODO: push light into things as well, so they get added
    // as geometry top the scene
  }
};
