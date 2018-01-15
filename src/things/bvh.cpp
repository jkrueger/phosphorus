#include "bvh.hpp"
#include "precision.hpp"
#include "shading.hpp"

#include <algorithm>

static const uint8_t MAX_PRIMS_IN_NODE = 4;
static const uint8_t NUM_SPLIT_BUCKETS = 12;

struct node_t {
  aabb_t   bounds;
  uint32_t offset;
  uint16_t num;
  uint8_t  axis;
  uint8_t  padding; // pad to 32 byte

  inline node_t(const aabb_t& b, uint32_t o, uint16_t n)
    : bounds(b), offset(o), num(n)
  {}

  inline node_t(uint8_t axis)
    : offset(0), axis(axis), num(0)
  {}

  inline bool is_leaf() const {
    return num > 0;
  }
};

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
  std::vector<node_t> nodes;

  uint32_t num_nodes;
  uint32_t max_depth;
  uint32_t lonely;

  impl_t()
    : num_nodes(0), max_depth(0), lonely(0)
  {}

  template<typename... Args>
  uint32_t make_node(const Args& ...args) {
    nodes.emplace_back(args...);
    return num_nodes++;
  }

  inline const node_t* resolve(uint32_t node) const {
    return &nodes[node];
  }

  inline node_t* resolve(uint32_t node) {
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

    split(start, end, infos, unsorted, 0);
  }

  uint32_t split(
    uint32_t start, uint32_t end,
    std::vector<build_info_t>& infos,
    const std::vector<typename T::p>& unsorted,
    uint32_t depth) {
    uint32_t out;

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
	float_t  best_cost   = std::numeric_limits<float_t>::max();
	
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

	  float_t  split_cost   = std::numeric_limits<float_t>::max();
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
	    out = make_node(best_axis);

	    auto near = split(start, mid, infos, unsorted, depth+1);
	    auto far  = split(mid, end, infos, unsorted, depth+1);
	  
	    node_t* a      = resolve(near);
	    node_t* b      = resolve(far);
	    node_t* parent = resolve(out);
	    parent->offset = far;
	    parent->bounds = bounds::merge(a->bounds, b->bounds);
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
    << "Finished building BVH. Extent: "
    << impl->nodes[0].bounds
    << std::endl
    << "With number of nodes: " << impl->num_nodes
    << ", depth=" << impl->max_depth
    << ", lonely=" << impl->lonely
    << std::endl;
}

template<typename T>
bool bvh_t<T>::intersect(const ray_t& ray, shading_info_t& info) const {
  static thread_local uint32_t stack[128];

  const float_t ood[] =
    { 1.0f/ray.direction.x,
      1.0f/ray.direction.y,
      1.0f/ray.direction.z,
      0.0f };

  bool hit_anything = false;
  if (impl->num_nodes > 0) {
    // setup traverse stack
    auto top = 1;
    stack[0] = 0;
    while (top > 0) {
      auto cur  = stack[--top];
      auto node = impl->resolve(cur);
      if (node->is_leaf()) {
	for (int i=node->offset; i<(node->offset+node->num); ++i) {
	  if (impl->things[i]->intersect(ray, info)) {
	    hit_anything = true;
	  }
	}
      }
      else {
	float_t d0, d1;
	bool hita = impl->resolve(cur+1)->bounds.intersect(ray, ood, d0);
	bool hitb = impl->resolve(node->offset)->bounds.intersect(ray, ood, d1);
	
	hita = hita && d0 <= info.d;
	hitb = hitb && d1 <= info.d;
	
	if (hita && hitb) {
	  if (d0 <= d1) {
	    stack[top++] = node->offset;
	    stack[top++] = cur + 1;
	  }
	  else {
	    stack[top++] = cur + 1;
	    stack[top++] = node->offset;
	  }
	}
	else if (hita) {
	  stack[top++] = cur + 1;
	}
	else if (hitb) {
	  stack[top++] = node->offset;
	}
      }
    }
  }
  return hit_anything;
}

template class bvh_t<thing_t>;
template class bvh_t<triangle_t>;
