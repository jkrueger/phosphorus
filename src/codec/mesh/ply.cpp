#include "ply.hpp"
#include "precision.hpp"

#include <rply.h>

int add_vertex(p_ply_argument argument) {
  long index, axis;
  mesh_t* m;
  ply_get_argument_user_data(argument, (void**) &m, &axis);
  ply_get_argument_element(argument, NULL, &index);
  float_t v = ply_get_argument_value(argument);
  //if (axis == 2) {
  //   v = v * -1.0;
  //}
  mesh_t::vertices[m->index_vertices+index].v[axis] = v * 10.0;
  return 1;
}

int add_face(p_ply_argument argument) {
  long length, index, vertex;
  mesh_t* m;
  ply_get_argument_user_data(argument, (void**) &m, NULL);
  ply_get_argument_element(argument, NULL, &index);
  ply_get_argument_property(argument, NULL, &length, &vertex);
  auto v = ply_get_argument_value(argument);
  if (vertex >= 0) {
    mesh_t::faces[m->index_faces+(index*3)+vertex] = v;
  }
  return 1;
}

mesh_t::p codec::mesh::ply::load(const std::string& path, const material_t::p& default_material) {
  auto mesh = new mesh_t(default_material);

  if (auto ply = ply_open(path.c_str(), NULL, 0, NULL)) {
    if (!ply_read_header(ply)) {
      throw std::runtime_error("Failed to open mesh header: " + path);
    }

    mesh->num_vertices = ply_set_read_cb(ply, "vertex", "x", add_vertex, mesh, 2);
    ply_set_read_cb(ply, "vertex", "y", add_vertex, mesh, 1);
    ply_set_read_cb(ply, "vertex", "z", add_vertex, mesh, 0);

    mesh_t::vertices.resize(mesh_t::vertices.size()+mesh->num_vertices);

    mesh->num_faces = ply_set_read_cb(ply, "face", "vertex_indices", add_face, mesh, 0);

    mesh_t::faces.resize(mesh_t::faces.size()+(mesh->num_faces*3));

    if (!ply_read(ply)) {
      return nullptr;
    }

    ply_close(ply);

    mesh->compute_normals();
  }
  else {
    throw std::runtime_error("Cam't open mesh: " + path);
  }

  return mesh_t::p(mesh);
}
