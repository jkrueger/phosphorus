#include "scene.hpp"
#include "traversal/bvh.hpp"

template<typename T>
scene_t<T>::~scene_t()
{
  for (auto& l : lights) {
    delete l;
  }

  for (auto& m : meshes) {
    delete m;
  }

  for (auto& m : materials) {
    delete m;
  }
}

template<typename T>
void scene_t<T>::preprocess() {
  std::vector<triangle_t::p> triangles;
  for (const auto& thing: meshes) {
    thing->tesselate(triangles);
  }

  accel.build(triangles);
}

template<typename T>
bool scene_t<T>::intersect(segment_t& segment, float_t& d) const {
  stats->rays++;
  return accel.intersect(segment, d);
}

template<typename T>
void scene_t<T>::intersect(segment_t* stream, const active_t& active) const {
  accel.intersect(stream, active);
  stats->rays += active.num;
}

template<typename T>
bool scene_t<T>::occluded(segment_t& segment, const vector_t& dir, float_t d) const {
  stats->rays++;
  return accel.occluded(segment, dir, d);
}

template<typename T>
void scene_t<T>::occluded(occlusion_query_t* stream, const active_t& active) const {
  accel.occluded(stream, active);
  stats->rays += active.num;
}

template class scene_t<mesh_bvh_t>;
