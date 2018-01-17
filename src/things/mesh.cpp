#include "mesh.hpp"
#include "shading.hpp"

// std::vector<vector_t> mesh_t::vertices;
// std::vector<vector_t> mesh_t::normals;
// std::vector<uint32_t> mesh_t::faces;

void mesh_t::tesselate(std::vector<triangle_t::p>& out) const {
  for (auto i=index_faces; i<index_faces+(num_faces*3); i+=3) {
    out.push_back(
      triangle_t::p(
        new triangle_t(
	  this,
	  mesh_t::faces[i  ],
	  mesh_t::faces[i+1],
	  mesh_t::faces[i+2])));
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

bool mesh_t::intersect(const ray_t& ray, shading_info_t& info) const {
  throw std::runtime_error("Can't intersect meshes directly");
}

void mesh_t::sample(const vector_t&, const sample_t*, sampled_vector_t*, uint32_t) const {
  throw std::runtime_error("Sampling of meshes not implemented");
}

triangle_t::triangle_t(const mesh_t* m, int a, int b, int c)
  : mesh(m), a(a), b(b), c(c), shadable_t(m->position, m->material)
{}

bool triangle_t::intersect(const ray_t& ray, shading_info_t& info) const {
  vector_t v0v1 = mesh->vertex(b) - mesh->vertex(a);
  vector_t v0v2 = mesh->vertex(c) - mesh->vertex(a);
  vector_t p    = cross(ray.direction, v0v2);

  auto det = dot(v0v1, p);

  if (std::abs(det) < 0.00000001) {
    return false;
  }

  auto ood = 1.0 / det;

  vector_t t = ray.origin - mesh->vertices[a];

  auto u = dot(t, p) * ood;
  if (u < 0.0 || u > 1.0) {
    return false;
  }

  vector_t q = cross(t, v0v1);

  auto v = dot(ray.direction, q) * ood;
  if (v < 0.0 || (u + v) > 1.0) {
    return false;
  }

  auto d = dot(v0v2, q) * ood;
  if (d < 0.0) {
    return false;
  }

  return info.update(ray, d, this, u, v);
}

void triangle_t::sample(const vector_t&, const sample_t*, sampled_vector_t*, uint32_t) const {
  throw std::runtime_error("Sampling of triangles not implemented");
}

void triangle_t::shading_parameters(shading_info_t& info, const vector_t&, float_t u, float_t v) const {
  auto w = (1 - u - v);
  info.n = u * mesh->normal(a) + v * mesh->normal(b) + w * mesh->normal(c);
  info.n.normalize();
}

aabb_t triangle_t::bounds() const {
  aabb_t out;
  bounds::merge(out, mesh->vertex(a));
  bounds::merge(out, mesh->vertex(b));
  bounds::merge(out, mesh->vertex(c));
  return out;
}
