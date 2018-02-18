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

  template<typename U>
  static inline bool intersect(
      traversal_ray_t<U>& ray
    , const moeller_trumbore_t<build::MAX_PRIMS_IN_NODE>& tris)
  {
    return tris.intersect(ray);
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

  bool intersect(segment_t& segment, const vector_t& dir, bool occlusion_query) {
    static thread_local node_ref_t stack[128];

    uint32_t indices[] = {
      0, 8, 16, 24, 32, 40
    };

    //if (dir.x < 0.0) { std::swap(indices[0], indices[3]); }
    //if (dir.y < 0.0) { std::swap(indices[1], indices[4]); }
    //if (dir.z < 0.0) { std::swap(indices[2], indices[5]); }

    traversal_ray_t<segment_t> tray(segment.p, dir, &segment);

    bool hit_anything = false;

    auto top = 1;
    stack[0].offset = 0;
    stack[0].flags  = 0;
    stack[0].d = std::numeric_limits<float>::lowest();

    while (top > 0) {
      auto cur = stack[--top];
      if (cur.d > segment.d) {
	continue;
      }

      while (cur.flags == 0) {
	auto node = resolve(cur.offset);
	__aligned(64) auto bounds = bounds::load<8>(node->bounds, indices);
	
	float8_t dist;
	auto mask = float8::movemask(bounds::intersect_all<8>(
          tray.origin, tray.ood, tray.d,
	  bounds, dist));

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
	if (accelerator_t<T>::intersect(tray, things[cur.offset])) {
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

  template<typename Stream>
  void intersect(Stream* stream, const active_t& active) const {
    static thread_local stream::lanes_t lanes;
    static thread_local stream::task_t  tasks[256];

    static thread_local __attribute__((aligned (64))) traversal_ray_t<Stream> rays[4096];

    auto ray = rays;
    for (auto i=0; i<active.num; ++i, ++ray) {
      auto  index   = active.segment[i];
      auto& segment = stream[index];
      if (!segment.masked()) {
	segment.kill();
	// avoid copying of data and call constructor directly
	new(ray) traversal_ray_t<Stream>(segment.p, segment.wi, &segment);
	push(lanes, 0, i);
      }
    }

    // all rays were masked
    if (lanes.num[0] == 0) {
      return;
    }

    auto zero = float8::load(0.0f);

    auto top = 0;
    push(tasks, top, 0, lanes.num[0], 0, 0);

    while (top > 0) {
      auto& cur = tasks[--top];

      if (!cur.is_leaf()) {
	auto node = resolve(cur.offset);
	auto todo = pop(lanes, cur.lane, cur.num_rays);

	uint32_t indices[] = {
	  0, 8, 16, 24, 32, 40
	};

	uint32_t num_active[8] = {[0 ... 7] = 0};

	__aligned(64) auto bounds = bounds::load<8>(node->bounds, indices);

	auto length = zero;
	auto end    = todo + cur.num_rays;
	while (todo != end) {
	  const auto& ray = rays[*todo];

	  if (Stream::stop_on_first_hit && ray.segment->alive()) {
	    ++todo;
	    continue;
	  }

	  float8_t dist;

	  auto hits = bounds::intersect_all<8>(
            ray.origin, ray.ood, ray.d,
	    bounds, dist);

	  length = float8::add(length, float8::mand(dist, hits));

	  auto mask = float8::movemask(hits);

	  // push ray into lanes for intersected nodes
	  while(mask != 0) {
	    auto x = __bscf(mask);
	    num_active[x]++;
	    push(lanes, x, *todo);
	  }

	  ++todo;
	}
	
	uint32_t ids[8];

	float dists[8];
	float8::store(length, dists);
	
	// TODO: collect stats on lane utilization to see if efficiently sorting
	// for smaller 'n' makes sense
	
	auto n=0;
	for (auto i=0; i<8; ++i) {
	  auto num = num_active[i];
	  if (num > 0) {
	    auto d = dists[i];
	    ids[n] = i;
	    for(auto j=n; j>0; --j) {
	      if (d < dists[ids[j]]) {
		auto& a = ids[j], &b = ids[j-1];
		a = a ^ b;
		b = a ^ b;
		a = a ^ b;
	      }
	    }
	    ++n;
	  }
	}

	for (auto i=0; i<n; ++i) {
	  auto num = num_active[ids[i]];
	  auto is_leaf = node->is_leaf(ids[i]) ? 1 : 0;
	  push(tasks, top, node->offset[ids[i]], num, ids[i], is_leaf);
	}
      }
      else {
	auto todo = pop(lanes, cur.lane, cur.num_rays);
	auto end  = todo + cur.num_rays;
	do {
	  if (accelerator_t<T>::intersect(
	        rays[*todo]
	      , things[cur.offset])) {
	    rays[*todo].segment->revive();
	  }
	} while(++todo != end);
      }
    }
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
bool bvh_t<T>::intersect(segment_t& segment, float_t& d) const {
  segment.d = d;
  auto ret = impl->intersect(segment, segment.wi, false);
  d = segment.d;
  return ret;
}

template<typename T>
void bvh_t<T>::intersect(segment_t* stream, const active_t& active) const {
  impl->intersect(stream, active);
}

template<typename T>
bool bvh_t<T>::occluded(segment_t& segment, const vector_t& dir, float_t d) const {
  segment.d = d;
  return impl->intersect(segment, dir, true);
}

template<typename T>
void bvh_t<T>::occluded(occlusion_query_t* stream, const active_t& active) const {
  impl->intersect(stream, active);
}

template class bvh_t<triangle_t>;
