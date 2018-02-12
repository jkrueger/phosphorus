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
#include "bvh/stacks.hpp"

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
    float_t& d,
    segment_t& s) {

    return tris.intersect(ray, d, s);
  }

  static inline uint32_t insert_things(
    uint32_t start, uint32_t end,
    const std::vector<build::primitive_t>& primitives,
    const std::vector<typename triangle_t::p>& unsorted,
    storage_t& things) {

    triangle_t::p tris[build::MAX_PRIMS_IN_NODE];

    for (auto j=0; j<(end-start); ++j) {
      tris[j] = unsorted[primitives[start+j].index];
    }

    things.emplace_back(tris, (end-start));

    return things.size()-1;
  }
};

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

  bool intersect(segment_t& segment, const vector_t& dir, float_t& d, bool occlusion_query) {
    static thread_local node_ref_t stack[128];

    uint32_t indices[] = {
      0, 8, 16, 24, 32, 40
    };

    if (dir.x < 0.0) { std::swap(indices[0], indices[3]); }
    if (dir.y < 0.0) { std::swap(indices[1], indices[4]); }
    if (dir.z < 0.0) { std::swap(indices[2], indices[5]); }

    traversal_ray_t tray(segment.p, dir, d);

    bool hit_anything = false;

    auto top = 1;
    stack[0].offset = 0;
    stack[0].flags  = 0;
    stack[0].d = std::numeric_limits<float>::lowest();

    while (top > 0) {
      auto cur = stack[--top];
      if (cur.d > d) {
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
	if (accelerator_t<T>::intersect(tray, things[cur.offset], d, segment)) {
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

  uint32_t intersect(segment_t* stream, uint32_t num) const {
    stream::lanes_t lanes;
    stream::task_t  tasks[128];

    traversal_ray_t rays[num];

    auto dmax    = std::numeric_limits<float>::max();
    auto segment = stream;
    for (auto i=0; i<num; ++i, ++segment) {
      if (segment->alive()) {
	rays[i] = traversal_ray_t(segment->p, segment->wi, dmax);
	push(lanes, 0, i);
      }
    }

    uint32_t ret = lanes.num[0];

    auto zero = float8::load(0.0f);

    auto top = 1;
    push(tasks, top, 0, lanes.num[0], 0);

    while (top > 0) {
      auto cur  = tasks[--top];
      auto node = resolve(cur.offset);

      if (!node->is_leaf(cur.lane)) {
	auto todo   = pop(lanes, cur.lane, cur.num_rays);
	auto bounds = bounds::load(node);

	auto length = zero;
	for (auto i=0; i<cur.num_rays; ++i, ++todo) {
	  auto& ray = rays[*todo];

	  float8_t dist;

	  auto mask = bounds::intersect_all<8>(
            ray.origin, ray.ood, ray.d,
	    bounds, nullptr,
	    dist);

	  length = float8::add(length, float8::mand(dist, mask));

	  // push ray into lanes for intersected nodes
	  for (auto i=0; i<8; ++i) {
	    if (mask & (1 << i) != 0) {
	      push(lanes, i, ray);
	    }
	  }
	}

	// TODO: sort by accumulated distance
	uint32_t ids[] = {0, 1, 2, 3, 4, 5, 6, 7};
	for (auto i=0; i<8; ++i) {
	  push(tasks, top, node->offset[ids[i]], lanes.num[ids[i]], i);
	}
      }
      else {
	auto todo = pop(lanes, cur.lane, cur.num_rays);

	for (auto i=0; i<cur.num_rays; ++i, ++todo) {
	  accelerator_t<T>::intersect(rays[*todo], things[cur.offset], d, segment[*todo]);
	}
      }
    }

    return ret;
  }
};

template<typename T>
bvh_t<T>::bvh_t()
  : impl(new impl_t())
{}

template<typename T>
void bvh_t<T>::build(const std::vector<triangle_t::p>& things) {
  printf("ray=%u\n", sizeof(traversal_ray_t));
  printf("triangles=%u\n", sizeof(moeller_trumbore_t<8>));

  impl->build(things);

  std::clog
    << "Finished building BVH."
    << impl->bounds
    << std::endl
    << "With number of nodes: " << impl->things.size()
    << std::endl;
}

template<typename T>
bool bvh_t<T>::intersect(segment_t& segment, float_t& d) const {
  return impl->intersect(segment, segment.wi, d, false);
}

template<typename T>
bool bvh_t<T>::occluded(segment_t& segment, const vector_t& dir, float_t d) const {
  return impl->intersect(segment, dir, d, true);
}

template class bvh_t<triangle_t>;
