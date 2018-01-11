#include "ply.hpp"

#include <rply.h>

int add_vertex(p_ply_argument argument) {
  long index, axis;
  mesh_t* m;
  ply_get_argument_user_data(argument, (void**) &m, &axis);
  m->vertices[index].v[axis] = ply_get_argument_value(argument);
  return 1;
}

int add_face(p_ply_argument argument) {
  long length, index, vertex;
  mesh_t* m;
  ply_get_argument_user_data(argument, (void**) &m, NULL);
  ply_get_argument_element(argument, NULL, &index);
  ply_get_argument_property(argument, NULL, &length, &vertex);
  auto v = ply_get_argument_value(argument);
  switch (index) {
  case 0: m->triangles[index].a = v; break;
  case 1: m->triangles[index].b = v; break;
  case 2: m->triangles[index].c = v; break;
  }
  return 1;
}

mesh_t::p codec::mesh::ply::load(const std::string& path, const material_t::p& default_material) {
  auto mesh = new mesh_t({0,0,0}, default_material);

  if (auto ply = ply_open(path.c_str(), NULL, 0, mesh)) {
    if (!ply_read_header(ply)) {
      throw std::runtime_error("Failed to open mesh header: " + path);
    }

    auto nvertices = ply_set_read_cb(ply, "vertex", "x", add_vertex, NULL, 0);
    ply_set_read_cb(ply, "vertex", "y", add_vertex, NULL, 1);
    ply_set_read_cb(ply, "vertex", "z", add_vertex, NULL, 2);

    mesh->vertices.reserve(nvertices);
    
    auto nfaces = ply_set_read_cb(ply, "face", "vertex_indices", add_face, NULL, 0);

    mesh->triangles.resize(nfaces);

    if (!ply_read(ply)) {
      return nullptr;
    }

    ply_close(ply);
  }
  else {
    throw std::runtime_error("Cam't open mesh: " + path);
  }

  return mesh_t::p(mesh);
}
