#include "bvh.hpp"
#include "shading.hpp"

#include <algorithm>

static const uint8_t MAX_PRIMS_IN_NODE = 4;
static const uint8_t NUM_SPLIT_BUCKETS = 12;

struct node_t {

  node_t* a, *b;

  aabb_t   bounds;
  uint32_t offset;
  uint32_t num;
  uint8_t  axis;

  inline node_t(const aabb_t& b, uint32_t o, uint32_t n)
    : bounds(b), offset(o), num(n)
  {}

  inline node_t(uint8_t axis, node_t* a, node_t* b)
    : a(a), b(b), bounds(bounds::merge(a->bounds, b->bounds)), offset(0), num(0), axis(axis)
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

struct bvh_t::impl_t {
  std::vector<thing_t::p> things;
  node_t*                 root;

  void insert_things(
    uint32_t start, uint32_t end,
    std::vector<build_info_t>& infos,
    const std::vector<thing_t::p>& unsorted) {

    for (auto i=start; i<end; ++i) {
      things.push_back(unsorted[infos[i].index]);
    }
  }

  node_t* build(
    uint32_t start, uint32_t end,
    std::vector<build_info_t>& infos,
    const std::vector<thing_t::p>& unsorted) {
    node_t* out;

    auto n = (end - start);
    if (n == 1) {
      insert_things(start, end, infos, unsorted);
      out = new node_t(infos[start].bounds, start, n);
    }
    else {
      aabb_t node_bounds, centroid_bounds;

      for (auto i=start; i<end; ++i) {
	node_bounds = bounds::merge(node_bounds, infos[i].bounds);
	bounds::merge(centroid_bounds, infos[i].centroid);
      }
      auto axis = centroid_bounds.dominant_axis();

      if (centroid_bounds.empty_on(axis)) {
	out = new node_t(node_bounds, start, n);
      }
      else {
	auto mid = (start + end) / 2;
	if (n <= 2) {
	  insert_things(start, end, infos, unsorted);
	  out = new node_t(node_bounds, start, n);
	}
	else {
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
	  auto    split_left   = 0;
	  auto    split_right  = 0;

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
	      split_left   = left;
	      split_right  = right;
	    }
	  }

	  double leaf_cost = n;
	  if (split_left != n && split_right != n &&
	      (n > MAX_PRIMS_IN_NODE || split_cost < leaf_cost)) {
	    auto p =
	      std::partition(
	        &infos[start], &infos[end-1] + 1,
		[=](const build_info_t& info) {
		  auto offset = bounds::offset(centroid_bounds, info.centroid).v[axis];
		  auto bucket =
		    std::min(
		      (int) (NUM_SPLIT_BUCKETS * offset),
		      NUM_SPLIT_BUCKETS-1);
		  return bucket <= split_bucket;
		});
	    mid = p - &infos[0];
	    out =
	      new node_t(
	        axis,
		build(start, mid, infos, unsorted),
		build(mid, end, infos, unsorted));
	  }
	  else {
	    insert_things(start, end, infos, unsorted);
	    out = new node_t(node_bounds, start, n);
	  }
	}
      }
    }

    return out;
  }
};

bvh_t::bvh_t()
  : impl(new impl_t())
{}

void bvh_t::build(const std::vector<thing_t::p>& things) {
  std::vector<build_info_t> infos(things.size());

  for (uint32_t i=0; i<things.size(); ++i) {
    infos[i] = build_info_t(i, things[i]->bounds());
  }
  impl->root = impl->build(0, (uint32_t) infos.size(), infos, things);
}

bool bvh_t::intersect(const ray_t& ray, shading_info_t& info) const {
  static thread_local node_t* stack[64];

  bool hit_anything = false;
  if (impl->root) {
    auto cur  = 0;
    auto node = const_cast<node_t*>(impl->root);
    while (true) {
      if (node->bounds.intersect(ray)) {
	if (node->is_leaf()) {
	  for (int i=node->offset; i<(node->offset+node->num); ++i) {
	    if (impl->things[i]->intersect(ray, info)) {
	      hit_anything = true;
	    }
	  }
	  if (cur == 0) { break; }
	  node = stack[--cur];
	}
	else {
	  if (ray.direction.v[node->axis]) {
	    stack[cur++] = node->b;
	    node = node->a;
	  }
	  else {
	    stack[cur++] = node->a;
	    node = node->b;
	  }
	}
      }
      else {
	if (cur == 0) { break; }
	node = stack[--cur];
      }
    }
  }

  return hit_anything;
}
