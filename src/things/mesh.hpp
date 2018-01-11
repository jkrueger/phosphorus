#pragma once

#include "thing.hpp"

struct mesh_t;

struct triangle_t : public shadable_t {
  typedef std::shared_ptr<triangle_t> p;
  
  const mesh_t* mesh;
  uint32_t a, b, c;
  // TODO: precomputed values

  triangle_t(const mesh_t* m, int a, int b, int c);

  bool intersect(const ray_t& ray, shading_info_t& info) const;

  void sample(const vector_t&, const sample_t*, sampled_vector_t*, uint32_t) const;

  void shading_parameters(shading_info_t&, const vector_t&, double u, double v) const;

  aabb_t bounds() const;
};

struct mesh_t : public shadable_t {
  typedef std::shared_ptr<mesh_t> p;

  // global buffers for mesh data. we can definetely do better here but this
  // is as simple a pool as it gets for now
  static std::vector<vector_t> vertices;
  static std::vector<vector_t> normals;
  static std::vector<uint32_t> faces;

  uint32_t index_vertices;
  uint32_t index_faces;

  uint32_t num_vertices;
  uint32_t num_faces;

  aabb_t _bounds;

  mesh_t(const vector_t& p, const material_t::p& m)
    : shadable_t(p, m),
      index_vertices(vertices.size()),
      index_faces(faces.size())
  {}

  void compute_normals();
  
  void tesselate(std::vector<triangle_t::p>&) const;

  bool intersect(const ray_t&, shading_info_t&) const;

  void sample(const vector_t&, const sample_t*, sampled_vector_t*, uint32_t) const;

  aabb_t bounds() const
  { return _bounds; }

  inline const vector_t& vertex(uint32_t index) const {
    return mesh_t::vertices[index_vertices+index];
  }

  inline const vector_t normal(uint32_t index) const {
    return mesh_t::normals[index_vertices+index];
  }
};
