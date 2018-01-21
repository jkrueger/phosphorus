#include "bvh.hpp"
#include "precision.hpp"
#include "shading.hpp"
#include "util/compiler.hpp"

#include <algorithm>

#include <string.h>

static const uint8_t MAX_PRIMS_IN_NODE = 4;
static const uint8_t NUM_SPLIT_BUCKETS = 12;

inline size_t __bsf(size_t v) {
  return _tzcnt_u64(v);
}

inline size_t __bscf(size_t& v) {
  size_t i = __bsf(v);
  v &= v-1;
  return i;
}

struct binary_node_t {
  aabb_t         bounds;
  binary_node_t* a, *b;
  uint32_t       offset;
  uint16_t       num;
  uint8_t        axis;

  inline binary_node_t(const aabb_t& b, uint32_t o, uint16_t n)
    : bounds(b), offset(o), num(n), a(nullptr), b(nullptr)
  {}

  inline binary_node_t(const aabb_t& bounds, uint8_t axis, binary_node_t* a, binary_node_t* b)
    : bounds(bounds), axis(axis), offset(0), num(0), a(a), b(b) 
  {}

  inline bool is_leaf() const {
    return num > 0;
  }
};

template<int N>
struct mbvh_node_t {
  // child node boundaries
  float    bounds[2*N*3];
  // offsts into child nodes, or pointers to primitives
  uint32_t offset[N];
  uint32_t axis[N-1];
  uint8_t  num[N];

  inline mbvh_node_t() {
    memset(bounds, 0, 2*N*3*4);
    memset(offset, 0, N*sizeof(uint32_t));
    memset(num, 0, N*sizeof(uint8_t));
  }

  inline bool is_leaf(uint32_t i) const {
    return num[i] > 0;
  }

  inline bool is_empty(uint32_t i) const {
    return num[i] == 0 && offset[i] == 0;
  }

  inline void set_offset(uint32_t i, uint32_t o) {
    offset[i] = o;
  }

  inline void set_leaf_size(uint32_t i, uint8_t size) {
    num[i] = size;
  }
};

typedef mbvh_node_t<4> quad_node_t;

struct build_info_t {
  uint32_t index;
  aabb_t   bounds;
  vector_t centroid;

  build_info_t()
  {}

  build_info_t(const build_info_t& cpy)
    : index(cpy.index), bounds(cpy.bounds), centroid(cpy.centroid)
  {}
  
  build_info_t(uint32_t i, const aabb_t& b)
    : index(i), bounds(b), centroid(b.centroid())
  {}
};

struct split_info_t {
  uint32_t count;
  aabb_t   bounds;

  inline split_info_t()
    : count(0)
  {}
};

struct traversal_ray_t {
  const vector4_t origin;
  const vector4_t direction;
  const vector4_t ood;
  float4_t d;
  const ray_t&    ray;

  inline traversal_ray_t(const ray_t& ray, float d)
    : ray(ray)
    , origin(ray.origin)
    , direction(ray.direction)
    , ood(vector_t(
        1.0f/ray.direction.x,
        1.0f/ray.direction.y,
        1.0f/ray.direction.z))
    , d(float4::load(d))
  {}
} __attribute__((aligned (16)));

struct moeller_trumbore_t {
  vector4_t e0;
  vector4_t e1;
  vector4_t v0;

  triangle_t::p triangles[MAX_PRIMS_IN_NODE];

  inline moeller_trumbore_t(const triangle_t::p* tris, uint32_t num) {
    vector_t
      ve0[MAX_PRIMS_IN_NODE],
      ve1[MAX_PRIMS_IN_NODE],
      vv0[MAX_PRIMS_IN_NODE];

    for (int i=0; i<num; ++i) {
      triangles[i] = tris[i];

      ve0[i] = triangles[i]->v1() - triangles[i]->v0();
      ve1[i] = triangles[i]->v2() - triangles[i]->v0();
      vv0[i] = triangles[i]->v0();
    }

    e0 = vector4_t(ve0);
    e1 = vector4_t(ve1);
    v0 = vector4_t(vv0);
  };

  inline bool intersect(traversal_ray_t& ray, shading_info_t& info) const {
    using namespace float4;
    using namespace vector4;

    const auto one = load(1.0f), zero = load(0.0f),
      peps = load(0.00000001f),
      meps = load(-0.00000001);

    const auto p   = cross(ray.direction, e1);
    const auto det = dot(e0, p);
    const auto ood = div(one, det);
    const auto t   = sub(ray.origin, v0);
    const auto q   = cross(t, e0);

    const auto u   = mul(dot(t, p), ood);
    const auto v   = mul(dot(ray.direction, q), ood);

    auto d = mul(dot(e1, q), ood);

    const auto xmask = mor(gt(det, peps), lt(det, meps));
    const auto umask = gte(u, zero);
    const auto vmask = mand(gte(v, zero), lte(add(u, v), one));
    const auto dmask = mand(gte(d, zero), lt(d, ray.d));

    auto mask = movemask(mand(mand(mand(vmask, umask), dmask), xmask));
    auto maskcpy = mask;

    bool ret = false;
    if (mask != 0) {

      float dists[4];
      float closest = std::numeric_limits<float>::max();

      store(d, dists);

      int idx = -1;
      while(mask != 0) {
	auto x = __bscf(mask);
	if (dists[x] < closest && triangles[3-x]) {
	  closest = dists[x];
	  idx = x;
	}
      }

      if (idx != -1) {
	float us[4];
	float vs[4];
	store(u, us); store(v, vs);
	ret = info.update(ray.ray, closest, triangles[3-idx].get(), us[idx], vs[idx]);

	d = _mm_min_ps(d, _mm_shuffle_ps(d, d, _MM_SHUFFLE(2, 1, 0, 3)));
        d = _mm_min_ps(d, _mm_shuffle_ps(d, d, _MM_SHUFFLE(1, 0, 3, 2)));

	ray.d = d;
      }
    }

    return ret;
  }
} __attribute__((aligned (16)));

template<typename T>
struct accelerator_t {
  typedef std::vector<typename T::p> storage_t;

  static inline bool intersect(
    traversal_ray_t& ray,
    const typename T::p& thing,
    shading_info_t& info) {

    return thing->intersect(ray.ray, info);
  }

  static inline uint32_t insert_things(
    uint32_t start, uint32_t end,
    std::vector<build_info_t>& infos,
    const std::vector<typename T::p>& unsorted,
    storage_t& things) {

    for (auto i=start; i<end; ++i) {
      things.emplace_back(unsorted[infos[i].index]);
    }

    return start;
  }
};

template<>
struct accelerator_t<triangle_t> {
  typedef std::vector<moeller_trumbore_t> storage_t;

  static inline bool intersect(
    traversal_ray_t& ray,
    const moeller_trumbore_t& tris,
    shading_info_t& info) {

    return tris.intersect(ray, info);
  }

  static inline uint32_t insert_things(
    uint32_t start, uint32_t end,
    std::vector<build_info_t>& infos,
    const std::vector<typename triangle_t::p>& unsorted,
    storage_t& things) {

    triangle_t::p tris[4];
    int i=0;
    for (auto j=0; j<(end-start); ++j, ++i) {
      tris[i] = unsorted[infos[start+j].index];
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

inline void push(
  node_ref_t* stack, int32_t& top, const float* const dists,
  const quad_node_t* node, uint32_t idx) {

  stack[top].offset = node->offset[idx];
  stack[top].flags  = node->num[idx];
  stack[top].d      = dists[idx];
  ++top;
}

template<typename T>
struct bvh_t<T>::impl_t {
  typename accelerator_t<T>::storage_t things;
  std::vector<quad_node_t> nodes;

  aabb_t   bounds;
  uint32_t num_nodes;
  uint32_t max_depth;
  uint32_t lonely;

  impl_t()
    : num_nodes(0), max_depth(0), lonely(0)
  {}

  template<typename... Args>
  binary_node_t* make_node(const Args& ...args) {
    num_nodes++;
    return new binary_node_t(args...);
  }

  inline const quad_node_t* resolve(uint32_t node) const {
    return &nodes[node];
  }

  inline quad_node_t* resolve(uint32_t node) {
    return &nodes[node];
  } 

  void build(
    uint32_t start, uint32_t end,
    std::vector<build_info_t>& infos,
    const std::vector<typename T::p>& unsorted) {

    auto root = split(start, end, infos, unsorted, 0);
    collapse(root);
    bounds = root->bounds;
  }

  binary_node_t* split(
    uint32_t start, uint32_t end,
    std::vector<build_info_t>& infos,
    const std::vector<typename T::p>& unsorted,
    uint32_t depth) {
    binary_node_t* out;

    auto n = (end - start);
    if (n == 1) {
      lonely++;
      auto offset = accelerator_t<T>::insert_things(start, end, infos, unsorted, things);
      out = make_node(infos[start].bounds, offset, 1);
    }
    else {
      aabb_t node_bounds, centroid_bounds;

      for (auto i=start; i<end; ++i) {
	node_bounds = bounds::merge(node_bounds, infos[i].bounds);
	bounds::merge(centroid_bounds, infos[i].centroid);
      }

      if (n <= MAX_PRIMS_IN_NODE) {
	auto offset = accelerator_t<T>::insert_things(start, end, infos, unsorted, things);
	out = make_node(node_bounds, offset, (uint16_t) n);
      }
      else {
	auto mid = (start + end) / 2;

	uint8_t best_axis   = 0;
	uint8_t best_bucket = 0;
	float_t best_cost   = std::numeric_limits<float_t>::max();
	
	//auto axis = centroid_bounds.dominant_axis();
	for (auto axis=0; axis<3; ++axis) {
	  split_info_t split_infos[NUM_SPLIT_BUCKETS];

	  for (auto i=start; i<end; ++i) {
	    auto offset = bounds::offset(centroid_bounds, infos[i].centroid).v[axis];
	    auto bucket =
	      std::min(
		(int) (NUM_SPLIT_BUCKETS * offset),
		NUM_SPLIT_BUCKETS-1);

	    split_infos[bucket].count++;
	    split_infos[bucket].bounds =
	      bounds::merge(
	        split_infos[bucket].bounds,
		infos[i].bounds);
	  }

	  float_t split_cost   = std::numeric_limits<float_t>::max();
	  uint8_t split_bucket = 0;

	  for (auto i=0; i<NUM_SPLIT_BUCKETS-1; ++i) {
	    aabb_t a, b;
	    auto left = 0; auto right = 0;
	    for (auto j=0; j<=i; ++j) {
	      a = bounds::merge(a, split_infos[j].bounds);
	      left += split_infos[j].count;
	    }

	    for (auto j=i+1; j<NUM_SPLIT_BUCKETS; ++j) {
	      b = bounds::merge(b, split_infos[j].bounds);
	      right += split_infos[j].count;
	    }

	    float_t cost =
	      1.0f + (left * a.area() + right * b.area()) / node_bounds.area();
	    if (cost < split_cost) {
	      split_cost   = cost;
	      split_bucket = i;
	    }
	  }

	  if (split_cost < best_cost) {
	    best_axis   = axis;
	    best_cost   = split_cost;
	    best_bucket = split_bucket;
	  }
	}

	if (centroid_bounds.empty_on(best_axis)) {
	  auto offset = accelerator_t<T>::insert_things(start, end, infos, unsorted, things);
	  out = make_node(node_bounds, offset, (uint16_t) n);
	}
	else {
	  float_t leaf_cost = n;
	  if ((n > MAX_PRIMS_IN_NODE || best_cost < leaf_cost)) {
	    auto p =
	      std::partition(
	        &infos[start], &infos[end-1] + 1,
		[=](const build_info_t& info) {
		  auto offset = bounds::offset(centroid_bounds, info.centroid).v[best_axis];
		  auto bucket =
		  std::min(
		    (int) (NUM_SPLIT_BUCKETS * offset),
		    NUM_SPLIT_BUCKETS-1);
		  return bucket <= best_bucket;
		});

	    mid = p - &infos[0];

	    auto near = split(start, mid, infos, unsorted, depth+1);
	    auto far  = split(mid, end, infos, unsorted, depth+1);

	    out = make_node(bounds::merge(near->bounds, far->bounds), best_axis, near, far);
	  }
	  else {
	    auto offset = accelerator_t<T>::insert_things(start, end, infos, unsorted, things);
	    out = make_node(node_bounds, offset, (uint16_t) n);
	  }
	}
      }
    }
    max_depth = std::max(max_depth, depth);
    return out;
  }

  uint32_t collapse(binary_node_t* root) {
    auto offset = nodes.size();
    nodes.emplace_back();

    auto left  = root->a;
    auto right = root->b;

    binary_node_t* candidates[] = {
      left->a  ? left->a  : left,
      left->a  ? left->b  : nullptr,
      right->a ? right->a : right,
      right->a ? right->b : nullptr
    };

    for (int i=0; i<4; ++i) {
      if (candidates[i]) {
	quad_node_t* node = &nodes[offset];
	
	node->bounds[i     ] = candidates[i]->bounds.min.x;
	node->bounds[4  + i] = candidates[i]->bounds.min.y;
	node->bounds[8  + i] = candidates[i]->bounds.min.z;
	node->bounds[12 + i] = candidates[i]->bounds.max.x;
	node->bounds[16 + i] = candidates[i]->bounds.max.y;
	node->bounds[20 + i] = candidates[i]->bounds.max.z;

	if (candidates[i]->is_leaf()) {
	  node->set_offset(i, candidates[i]->offset);
	  node->set_leaf_size(i, candidates[i]->num);
	}
	else {
	  uint32_t child = collapse(candidates[i]);
	  nodes[offset].set_offset(i, child);
	}
      }
    }
    return offset;
  }

  bool intersect(const ray_t& ray, shading_info_t& info, bool occlusion_query) {
    static thread_local node_ref_t stack[128];
    
    uint32_t indices[] = {
      0, 4, 8, 12, 16, 20
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
	
	float4_t dist;
	auto mask = bounds::intersect_all<4>(tray.origin, tray.ood, tray.d, node->bounds, indices, dist);
	if (mask == 0) {
	  break;
	}
	
	float dists[4];
	float4::store(dist, dists);
	
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

	  if (dists[b] < dists[a]) std::swap(b, a);
	  if (dists[d] < dists[c]) std::swap(d, c);
	  if (dists[c] < dists[a]) std::swap(c, a);
	  if (dists[d] < dists[b]) std::swap(d, b);
	  if (dists[c] < dists[b]) std::swap(c, b);

	  push(stack, top, dists, node, d);
	  push(stack, top, dists, node, c);
	  push(stack, top, dists, node, b);
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
void bvh_t<T>::build(const std::vector<typename T::p>& things) {
  std::vector<build_info_t> infos(things.size());
  for (uint32_t i=0; i<things.size(); ++i) {
    infos[i] = build_info_t(i, things[i]->bounds());
  }
  impl->build(0, (uint32_t) infos.size(), infos, things);

  std::clog
    << "Finished building BVH."
    << impl->bounds
    << std::endl
    << "With number of nodes: " << impl->nodes.size()
    << ", depth=" << impl->max_depth
    << ", lonely=" << impl->lonely
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

template class bvh_t<thing_t>;
template class bvh_t<triangle_t>;
