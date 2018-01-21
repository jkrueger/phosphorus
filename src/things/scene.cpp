#include "scene.hpp"
#include "bvh.hpp"

template<typename T>
void scene_t<T>::preprocess() {
  std::vector<triangle_t::p> triangles;
  for (auto& thing: things) {
    thing->tesselate(triangles);
  }

  accel.build(triangles);
}

template<typename T>
bool scene_t<T>::intersect(const ray_t& ray, shading_info_t& info) const {
  stats->rays++;
  return accel.intersect(ray, info);
}

template<typename T>
bool scene_t<T>::occluded(const ray_t& ray, float_t d) const {
  stats->rays++;
  return accel.occluded(ray, d);
}

template class scene_t<mesh_bvh_t>;
