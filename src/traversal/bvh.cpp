#include "bvh.hpp"
#include "precision.hpp"
#include "shading.hpp"
#include "math/aabb.hpp"
#include "util/compiler.hpp"

#include "aabb.hpp"
#include "ray.hpp"
#include "triangles.hpp"

#include "bvh/node.hpp"
#include "bvh/build.hpp"

#include <algorithm>

#include <string.h>

template<typename T>
struct accelerator_t
{};

template<>
struct accelerator_t<triangle_t> {
  typedef std::vector<moeller_trumbore_t<build::MAX_PRIMS_IN_NODE>> storage_t;

  static inline bool intersect(
    traversal_ray_t& ray,
    const moeller_trumbore_t<build::MAX_PRIMS_IN_NODE>& tris,
    shading_info_t& info) {

    return tris.intersect(ray, info);
  }

  static inline uint32_t insert_things(
    uint32_t start, uint32_t end,
    const std::vector<build::primitive_t>& primitives,
    const std::vector<typename triangle_t::p>& unsorted,
    storage_t& things) {

    triangle_t::p tris[build::MAX_PRIMS_IN_NODE];
    int i=0;
    for (auto j=0; j<(end-start); ++j, ++i) {
      tris[i] = unsorted[primitives[start+j].index];
    }

    things.emplace_back(tris, (end-start));

    return things.size()-1;
  }
};

struct node_ref_t {
  uint32_t offset : 28;
  uint32_t flags  : 4;
  float    d;
};

template<typename Node>
inline void push(
  node_ref_t* stack, int32_t& top, const float* const dists,
  const Node* node, uint32_t idx) {
  stack[top].offset = node->offset[idx];
  stack[top].flags  = node->num[idx];
  stack[top].d      = dists[idx];
  ++top;
}

template<typename T>
struct bvh_t<T>::impl_t {
  typedef typename accelerator_t<T>::storage_t storage_t;
  typedef octa_node_t node_t;

  storage_t           things;
  std::vector<node_t> nodes;

  aabb_t   bounds;
  uint32_t node_width;

  impl_t()
    : node_width(8)
  {}

  template<typename... Args>
  uint32_t make_node(const Args& ...args) {
    nodes.emplace_back(args...);
    return nodes.size()-1;
  }

  inline const node_t* resolve(uint32_t node) const {
    return &nodes[node];
  }

  inline node_t* resolve(uint32_t node) {
    return &nodes[node];
  }

  uint32_t insert_things(
      uint32_t start
    , uint32_t end
    , const std::vector<build::primitive_t>& primitives
    , const std::vector<typename T::p>& unsorted) {
    return accelerator_t<T>::insert_things(start, end, primitives, unsorted, things);
  }

  void build(const std::vector<typename T::p>& unsorted) {
    std::vector<build::primitive_t> primitives(unsorted.size());
    for (uint32_t i=0; i<unsorted.size(); ++i) {
      primitives[i] = {i, unsorted[i]->bounds()};
    }

    build::geometry_t geometry(primitives, 0, primitives.size());
    build::from(geometry, unsorted, *this);
  }

  bool intersect(const ray_t& ray, shading_info_t& info, bool occlusion_query) {
    static thread_local node_ref_t stack[128];

    uint32_t indices[] = {
      0, 8, 16, 24, 32, 40
    };

    if (ray.direction.x < 0.0) { std::swap(indices[0], indices[3]); }
    if (ray.direction.y < 0.0) { std::swap(indices[1], indices[4]); }
    if (ray.direction.z < 0.0) { std::swap(indices[2], indices[5]); }

    traversal_ray_t tray(ray, info.d);

    bool hit_anything = false;

    auto top = 1;
    stack[0].offset = 0;
    stack[0].flags  = 0;
    stack[0].d = std::numeric_limits<float>::lowest();

    while (top > 0) {
      auto cur = stack[--top];
      if (cur.d > info.d) {
	continue;
      }

      while (cur.flags == 0) {
	auto node = resolve(cur.offset);
	
	float8_t dist;
	auto mask = bounds::intersect_all<8>(
          tray.origin, tray.ood, tray.d,
	  node->bounds, indices,
	  dist);

	if (mask == 0) {
	  break;
	}
	
	float dists[8];
	float8::store(dist, dists);
	
	auto a = __bscf(mask);
	if (likely(mask == 0)) {
	  cur.offset = node->offset[a];
	  cur.flags  = node->num[a];
	  cur.d      = dists[a];
	  continue;
	}

	auto b = __bscf(mask);
	if (likely(mask == 0)) {
	  if (dists[a] < dists[b]) {
	    cur.offset = node->offset[a];
	    cur.flags  = node->num[a];
	    cur.d      = dists[a];

	    push(stack, top, dists, node, b);
	  }
	  else {
	    cur.offset = node->offset[b];
	    cur.flags  = node->num[b];
	    cur.d      = dists[b];

	    push(stack, top, dists, node, a);
	  }
	  continue;
	}

	auto c = __bscf(mask);
	if (likely(mask == 0)) {
	  if (dists[b] < dists[a]) std::swap(b, a);
	  if (dists[c] < dists[b]) std::swap(c, b);
	  if (dists[b] < dists[a]) std::swap(b, a);

	  push(stack, top, dists, node, c);
	  push(stack, top, dists, node, b);
	}
	else {
	  auto d = __bscf(mask);
	  if (mask == 0) {
	  
	    if (dists[b] < dists[a]) std::swap(b, a);
	    if (dists[d] < dists[c]) std::swap(d, c);
	    if (dists[c] < dists[a]) std::swap(c, a);
	    if (dists[d] < dists[b]) std::swap(d, b);
	    if (dists[c] < dists[b]) std::swap(c, b);

	    push(stack, top, dists, node, d);
	    push(stack, top, dists, node, c);
	    push(stack, top, dists, node, b);
	  }
	  else {
	    size_t indices[8] = {a,b,c,d,0,0,0,0};
	    int index = 4;
	    while (mask != 0) {
	      indices[index++] = __bscf(mask);
	    }
	    std::sort(indices, indices+index, [dists](size_t l, size_t r){
	      return dists[l] < dists[r];
	    });
	    for (auto i=1;i<index; ++i) {
	      push(stack, top, dists, node, indices[i]);
	    }

	    a = indices[0];
	  }    
	}

	cur.offset = node->offset[a];
	cur.flags  = node->num[a];
	cur.d      = dists[a];
      }

      if (cur.flags > 0) {
	if (accelerator_t<T>::intersect(tray, things[cur.offset], info)) {
	  hit_anything = true;
	}
      }

      if (hit_anything && occlusion_query) {
	// stop traversal if this is an occlusion query. we just want to
	// know if anything got hit
	break;
      }
    }
    
    return hit_anything;
  }
};

template<typename T>
bvh_t<T>::bvh_t()
  : impl(new impl_t())
{}

template<typename T>
void bvh_t<T>::build(const std::vector<triangle_t::p>& things) {

  impl->build(things);

  std::clog
    << "Finished building BVH."
    << impl->bounds
    << std::endl
    << "With number of nodes: " << impl->things.size()
    << std::endl;
}

template<typename T>
bool bvh_t<T>::intersect(const ray_t& ray, shading_info_t& info) const {
  return impl->intersect(ray, info, false);
}

template<typename T>
bool bvh_t<T>::occluded(const ray_t& ray, float_t d) const {
  shading_info_t info;
  info.d = d;
  return impl->intersect(ray, info, true);
}

template class bvh_t<triangle_t>;
