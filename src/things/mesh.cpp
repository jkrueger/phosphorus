#include "mesh.hpp"

void mesh_t::tesselate(const std::vector<thing_t::p>& out) const {
  for (auto& triangle : triangles) {
    out.push_back(triangle);
  }
}

bool mesh_t::intersect(const ray_t& ray, shading_info_t& info) const {
  bool hit_anything = false;
  for (auto& triangle : triangles) {
    bool hit_thing = triangle->intersect(ray, info);
    hit_anything |= hit_thing;
  }
  return hit_anything;
}

void mesh_t::sample(const vector_t&, const sample_t*, sampled_vector_t*, uint32_t) const {
  throw std::runtime_error("Sampling of meshes not implemented");
}

bool triangle_t::intersect(const ray_t& ray, shading_info_t& info) const {
  vector_t v0v1 = mesh->vertices[b] - mesh->vertices[a];
  vector_t v0v2 = mesh->vertices[c] - mesh->vertices[a];
  vector_t p    = cross(ray.direction, v0v2);

  auto det = dot(v0v1, p);

  if (std::abs(det) < 0.0001) {
    return false;
  }

  auto ood = 1.0 / d;

  vector_t t = ray.origin - mesh->vertices[a];

  auto u = t.dot(p) * ood;
  if (u < 0.0 || u > 1.0) {
    return false;
  }

  vector_t q = cross(ray.origin, v0v1);

  auto v = ray.direction.dot(q) * ood;
  if (v < 0.0 || (u + v) > 1.0) {
    return false;
  }

  return info.update(ray, dot(v0v2, q), mesh);
}

void triangle_t::sample(const vector_t&, const sample_t*, sampled_vector_t*, uint32_t) const {
  throw std::runtime_error("Sampling of triangles not implemented");
}

void triangle_t::shading_parameters(shading_info&, const vector_t&, double u, double v) const {
  auto w = (1 - u - v);
  info.n = u * mesh->normals[b] + v * mesh->normals[b] + w * mesh->normals[c];
  info.n.normalize();
}

aabb_t triangle_t::bounds() const {
  aabb_t out;
  bounds::merge(out, mesh->vertices[a]);
  bounds::merge(out, mesh->vertices[b]);
  bounds::merge(out, mesh->vertices[c]);
  return out;
}
