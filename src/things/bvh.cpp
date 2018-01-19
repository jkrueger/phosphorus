#include "bvh.hpp"
#include "precision.hpp"
#include "shading.hpp"
#include "util/compiler.hpp"

#include <algorithm>

static const uint8_t MAX_PRIMS_IN_NODE = 4;
static const uint8_t NUM_SPLIT_BUCKETS = 12;

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

template<typename T>
struct bvh_t<T>::impl_t {
  std::vector<typename T::p> things;
  std::vector<quad_node_t>   nodes;

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

  void insert_things(
    uint32_t start, uint32_t end,
    std::vector<build_info_t>& infos,
    const std::vector<typename T::p>& unsorted) {

    for (auto i=start; i<end; ++i) {
      things.push_back(unsorted[infos[i].index]);
    }
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
      insert_things(start, end, infos, unsorted);
      out = make_node(infos[start].bounds, start, 1);
    }
    else {
      aabb_t node_bounds, centroid_bounds;

      for (auto i=start; i<end; ++i) {
	node_bounds = bounds::merge(node_bounds, infos[i].bounds);
	bounds::merge(centroid_bounds, infos[i].centroid);
      }

      if (n <= 2) {
	insert_things(start, end, infos, unsorted);
	out = make_node(node_bounds, start, (uint16_t) n);
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
	  insert_things(start, end, infos, unsorted);
	  out = make_node(node_bounds, start, (uint16_t) n);
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
	    insert_things(start, end, infos, unsorted);
	    out = make_node(node_bounds, start, (uint16_t) n);
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

inline size_t __bsf(size_t v) {
  return _tzcnt_u64(v);
}

inline size_t __bscf(size_t& v) {
  size_t i = __bsf(v);
  v &= v-1;
  return i;
}

struct node_ref_t {
  uint32_t offset;
  uint32_t flags;
  float    d;
};

inline void push(node_ref_t* stack, int32_t& top, const float* const dists, const quad_node_t* node, uint32_t idx) {
  stack[top].offset = node->offset[idx];
  stack[top].flags  = node->num[idx];
  stack[top].d      = dists[idx];
  ++top;
}

template<typename T>
bool bvh_t<T>::intersect(const ray_t& ray, shading_info_t& info) const {
  static thread_local node_ref_t stack[128];

  const vector_t ood(
    1.0f/ray.direction.x,
    1.0f/ray.direction.y,
    1.0f/ray.direction.z);

  uint32_t indices[] = {
    0, 4, 8, 12, 16, 20
  };

  if (ray.direction.x < 0.0) { std::swap(indices[0], indices[3]); }
  if (ray.direction.y < 0.0) { std::swap(indices[1], indices[4]); }
  if (ray.direction.z < 0.0) { std::swap(indices[2], indices[5]); }

  const vector4_t dir(ood);
  const vector4_t org(ray.origin);

  bool hit_anything = false;
  if (impl->num_nodes > 0) {

    auto top = 1;
    stack[0].offset = 0;
    stack[0].flags  = 0;
    stack[0].d = std::numeric_limits<float>::max();

    while (top > 0) {
      auto cur = stack[--top];
      if (cur.d > info.d) {
	continue;
      }

      while (cur.flags == 0) {
	auto node = impl->resolve(cur.offset);
	
	float4_t dist;
	auto mask = bounds::intersect_all<4>(org, dir, node->bounds, indices, dist);
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
	for (int j=cur.offset; j<(cur.offset+cur.flags); ++j) {
	  if (impl->things[j]->intersect(ray, info)) {
	    hit_anything = true;
	  }
	}
      }
    }
  }
  return hit_anything;
}

template class bvh_t<thing_t>;
template class bvh_t<triangle_t>;
