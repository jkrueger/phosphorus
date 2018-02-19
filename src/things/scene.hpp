#pragma once

#include "mesh.hpp"
#include "light.hpp"
#include "thing.hpp"
#include "util/stats.hpp"

#include <vector>

template<typename Accel>
struct scene_t {
  Accel accel;

  std::vector<light_t::p>    lights;
  std::vector<mesh_t::p>     meshes;
  std::vector<material_t::p> materials;

  stats_t::p stats;

  scene_t(const stats_t::p& s)
    : stats(s)
  {}

  ~scene_t();

  void preprocess();

  bool intersect(segment_t& segment, float_t& d) const;

  void intersect(segment_t* stream, const active_t& active) const;

  bool occluded(segment_t& segment, const vector_t& dir, float_t d) const;

  void occluded(occlusion_query_t* stream, const active_t& active) const;

  inline void add(const mesh_t::p& thing) {
    meshes.push_back(thing);
  }

  inline void add(const light_t::p& light) {
    lights.push_back(light);
    // TODO: push light into things as well, so they get added
    // as geometry top the scene
  }

  inline void add(const material_t::p& material) {
    materials.push_back(material);
  }
};
