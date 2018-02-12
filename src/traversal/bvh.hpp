#pragma once

#include "shading.hpp"
#include "things/triangle.hpp"

#include <memory>

template<typename T>
struct bvh_t {
  typedef std::shared_ptr<bvh_t> p;
  
  struct impl_t;
  std::shared_ptr<impl_t> impl;

  bvh_t();

  /**
   * Preprocess the scene geometry to build the bounding volume
   * hierarchy
   *
   */
  void build(const std::vector<triangle_t::p>& things);

  /**
   * Find intersection for one segment. Returns the hitpoint distance
   * in 'd'
   *
   */
  bool intersect(segment_t&, float& d) const;

  /**
   * Find intersections for a stream of segments
   *
   */
  void intersect(segment_t* stream, uint32_t num) const;

  /**
   * Determine if the surface point described by segment is visibly from
   * the point 'p'
   *
   */
  bool occluded(segment_t& segment, const vector_t& dir, float_t d) const;
};

typedef bvh_t<triangle_t> mesh_bvh_t;
