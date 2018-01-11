#include "bvh.hpp"
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
	double  best_cost   = std::numeric_limits<double>::max();
	
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

	  double  split_cost   = std::numeric_limits<double>::max();
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

	    double cost =
	      1.0 + (left * a.area() + right * b.area()) / node_bounds.area();
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
	  double leaf_cost = n;
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
  std::cout << "build" << std::endl;
  for (uint32_t i=0; i<things.size(); ++i) {
    infos[i] = build_info_t(i, things[i]->bounds());
  }
  std::cout << "build2" << std::endl;
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

std::atomic<uint32_t> rays(0);
std::atomic<uint32_t> max_tris(0);
std::atomic<uint64_t> tot_tris(0);

template<typename T>
bool bvh_t<T>::intersect(const ray_t& ray, shading_info_t& info) const {
  uint32_t stack[128];

  double ood[] = { 1.0/ray.direction.x, 1.0/ray.direction.y, 1.0/ray.direction.z };

  uint32_t nodes = 0;
  uint32_t tris  = 0;

  bool hit_anything = false;
  if (impl->num_nodes > 0) {
    auto top = 0;
    auto cur = 0;
    while (true) {
      auto node = impl->resolve(cur);
      double d  = std::numeric_limits<double>::max();
      if (node->bounds.intersect(ray, ood, d) && d <= info.d) {
	if (node->is_leaf()) {
	  for (int i=node->offset; i<(node->offset+node->num); ++i) {
	    nodes = std::max(nodes, (uint32_t)node->num);
	    if (impl->things[i]->intersect(ray, info)) {
	      hit_anything = true;
	    }
	  }
	  if (top == 0) { break; }
	  cur = stack[--top];
	}
	else {
	  if (ray.direction.v[node->axis] < 0.0) {
	    stack[top++] = node->offset;
	    cur = cur + 1;
	  }
	  else {
	    stack[top++] = cur + 1;
	    cur = node->offset;
	  }
	}
      }
      else {
	if (top == 0) { break; }
	cur = stack[--top];
      }
    }
  }
  return hit_anything;
}

template class bvh_t<thing_t>;
template class bvh_t<triangle_t>;
