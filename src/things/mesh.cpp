#include "mesh.hpp"
#include "shading.hpp"

uint32_t mesh_t::ids = 0;

std::vector<vector_t> mesh_t::vertices;
std::vector<vector_t> mesh_t::normals;
std::vector<uint32_t> mesh_t::faces;

mesh_t::mesh_t(const material_t::p& m)
  : id(ids++)
  , index_vertices(vertices.size())
  ,  index_faces(faces.size())
  , material(m)
{}

mesh_t::mesh_t(
  const material_t::p& m, const vector_t* v, const vector_t* n,
  const uint32_t* f, uint32_t nv, uint32_t nf)
  : id(ids++)
  , index_vertices(vertices.size())
  , index_faces(faces.size())
  , material(m)
{
  num_vertices = nv;
  num_faces    = nf / 3;
    
  std::copy(v, v + nv, std::back_insert_iterator<decltype(vertices)>(vertices));
  std::copy(n, n + nv, std::back_insert_iterator<decltype(normals)>(normals));
  std::transform(
    f, f + nf, std::back_insert_iterator<decltype(faces)>(faces),
    [=](uint32_t f){
      return f;
    });
}

void mesh_t::tesselate(std::vector<triangle_t::p>& out) const {
  for (auto i=index_faces; i<index_faces+(num_faces*3); i+=3) {
    out.emplace_back(new triangle_t(this, i));
  }
}

void mesh_t::compute_normals() {
  normals.resize(normals.size()+num_vertices);

  for (auto i=0; i<num_faces*3; i+=3) {
    auto index = index_faces+i;
    uint32_t face[3] = {faces[index], faces[index+1], faces[index+2]};
    vector_t v0v1 = vertex(face[1]) - vertex(face[0]);
    vector_t v0v2 = vertex(face[2]) - vertex(face[0]);
    vector_t n = normalize(cross(v0v2,v0v1));
    for (auto j=0; j<3; ++j) {
      normals[index_vertices+face[j]] += n;
    }
  }

  for (auto i=index_vertices; i<index_vertices+num_vertices; ++i) {
    normals[i].normalize();
  }
}

triangle_t::triangle_t(const mesh_t* m, int id)
  : mesh(m), id(id)
{}

// bool triangle_t::intersect(const ray_t& ray, shading_info_t& info) const {
//   vector_t v0v1 = mesh->vertex(b) - mesh->vertex(a);
//   vector_t v0v2 = mesh->vertex(c) - mesh->vertex(a);
//   vector_t p    = cross(ray.direction, v0v2);

//   auto det = dot(v0v1, p);

//   if (std::abs(det) < 0.00000001) {
//     return false;
//   }

//   auto ood = 1.0 / det;

//   vector_t t = ray.origin - mesh->vertices[a];

//   auto u = dot(t, p) * ood;
//   if (u < 0.0 || u > 1.0) {
//     return false;
//   }

//   vector_t q = cross(t, v0v1);

//   auto v = dot(ray.direction, q) * ood;
//   if (v < 0.0 || (u + v) > 1.0) {
//     return false;
//   }

//   auto d = dot(v0v2, q) * ood;
//   if (d < 0.0) {
//     return false;
//   }

//   return info.update(ray, d, this, u, v);
// }

// void triangle_t::shading_parameters(shading_info_t& info, const vector_t&, float_t u, float_t v) const {
//   auto w = (1 - u - v);
//   info.n = w * mesh->normal(a) + u * mesh->normal(b) + v * mesh->normal(c);
//   info.n.normalize();
//   info.material = mesh->material;
// }

aabb_t triangle_t::bounds() const {
  aabb_t out;
  bounds::merge(out, mesh->vertex(mesh_t::faces[id  ]));
  bounds::merge(out, mesh->vertex(mesh_t::faces[id+1]));
  bounds::merge(out, mesh->vertex(mesh_t::faces[id+2]));
  return out;
}

vector_t triangle_t::v0() const {
  return mesh->vertex(mesh_t::faces[id]);
}

vector_t triangle_t::v1() const {
  return mesh->vertex(mesh_t::faces[id+1]);
}

vector_t triangle_t::v2() const {
  return mesh->vertex(mesh_t::faces[id+2]);
}
