#pragma once

#include "mesh.hpp"
#include "thing.hpp"
#include "triangle.hpp"
#include "material.hpp"
#include "shading.hpp"

#include <algorithm>

struct mesh_t : public thing_t {
  typedef mesh_t* p;

  // global buffers for mesh data. we can definetely do better here but this
  // is as simple a pool as it gets for now
  static std::vector<vector_t> vertices;
  static std::vector<vector_t> normals;
  static std::vector<uint32_t> faces;

  uint32_t id;
  
  uint32_t index_vertices;
  uint32_t index_faces;

  uint32_t num_vertices;
  uint32_t num_faces;

  material_t::p material;

  mesh_t(const material_t::p& m);

  mesh_t(
    const material_t::p& m, const vector_t* v, const vector_t* n,
    const uint32_t* f, uint32_t nv, uint32_t nf);

  void compute_normals();

  void tesselate(std::vector<triangle_t::p>&) const;

  inline const vector_t& vertex(uint32_t index) const {
    return mesh_t::vertices[index+index_vertices];
  }

  inline const vector_t& normal(uint32_t index) const {
    return mesh_t::normals[index+index_vertices];
  }

  inline const vector_t shading_normal(const segment_t& s) const {
    const auto& n0 = normal(mesh_t::faces[s.face  ]);
    const auto& n1 = normal(mesh_t::faces[s.face+1]);
    const auto& n2 = normal(mesh_t::faces[s.face+2]);

    auto w = 1 - s.u - s.v;
    auto n = (w*n0+s.u*n1+s.v*n2);

    n.normalize();

    return n;
  }
};
