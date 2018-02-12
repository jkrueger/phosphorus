#include "scene.hpp"
#include "traversal/bvh.hpp"

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
  return accel.intersect(segment, d);
  stats->rays++;
}

template<typename T>
void scene_t<T>::intersect(segment_t* stream, uint32_t num) const {
  accel.intersect(stream, num);
}

template<typename T>
bool scene_t<T>::occluded(segment_t& segment, const vector_t& dir, float_t d) const {
  stats->rays++;
  return accel.occluded(segment, dir, d);
}

template class scene_t<mesh_bvh_t>;
