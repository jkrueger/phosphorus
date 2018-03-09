#pragma once

#include "mesh.hpp"
#include "light.hpp"
#include "lights/environment.hpp"
#include "thing.hpp"
#include "util/stats.hpp"
#include "traversal/bvh.hpp"

#include <vector>

struct scene_t {
  virtual void add(const mesh_t::p&) = 0;

  virtual material_t::p material(uint32_t id) const = 0;
};

template<typename Accel>
struct scene_impl_t : public scene_t {
  Accel accel;

  std::vector<light_t::p>    lights;
  std::vector<mesh_t::p>     meshes;
  std::vector<material_t::p> materials;

  light::environment_t::p environment;

  stats_t::p stats;

  scene_impl_t(const stats_t::p& s)
    : stats(s)
  {}

  ~scene_impl_t();

  void preprocess();

  bool intersect(segment_t& segment, float_t& d) const;

  void intersect(segment_t* stream, const active_t& active) const;

  bool occluded(segment_t& segment, const vector_t& dir, float_t d) const;

  void occluded(occlusion_query_t* stream, const active_t& active) const;

  inline void add(const mesh_t::p& thing) {
    thing->id = meshes.size();
    meshes.push_back(thing);
  }

  inline void add(const light_t::p& light) {
    lights.push_back(light);
    // TODO: push light into things as well, so they get added
    // as geometry to the scene
  }

  inline void add(const material_t::p& material) {
    material->id = materials.size();
    materials.push_back(material);
  }

  inline material_t::p material(uint32_t id) const {
    return id < materials.size() ? materials[id] : nullptr;
  }

  inline bool has_environment() const {
    return environment;
  }

  inline color_t le(const segment_t& s) const {
    return environment->le(s);
  }
};

typedef scene_impl_t<mesh_bvh_t> mesh_scene_t; 
