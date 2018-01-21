#pragma once

#include "mesh.hpp"
#include "thing.hpp"
#include "triangle.hpp"
#include "material.hpp"

struct mesh_t : public thing_t {
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

  material_t::p material;

  mesh_t(const material_t::p& m)
    : index_vertices(vertices.size()),
      index_faces(faces.size()),
      material(m)
  {}

  void compute_normals();

  void tesselate(std::vector<triangle_t::p>&) const;

  inline const vector_t& vertex(uint32_t index) const {
    return mesh_t::vertices[index_vertices+index];
  }

  inline const vector_t normal(uint32_t index) const {
    return mesh_t::normals[index_vertices+index];
  }
};
