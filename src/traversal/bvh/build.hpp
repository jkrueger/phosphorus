#pragma once

#include "node.hpp"

#include "math/aabb.hpp"
#include "math/vector.hpp"

#include <algorithm>
#include <iostream>
#include <tuple>
#include <vector>

namespace build {
  static const uint8_t MAX_PRIMS_IN_NODE = 8;
  static const uint8_t NUM_SPLIT_BINS    = 12;

  struct primitive_t {
    uint32_t  index;    // an index into the real scene geometry
    aabb_t    bounds;   // the bounding box of the primitive
    vector_t  centroid; // the center of the bounding box

    inline primitive_t()
    {}

    inline primitive_t(const primitive_t& cpy)
      : index(cpy.index), bounds(cpy.bounds), centroid(cpy.centroid)
    {}

    inline primitive_t(uint32_t index, const aabb_t& bounds)
      : index(index), bounds(bounds), centroid(bounds.centroid())
    {}
  };

  /**
   * Build information about a subset of the primitives in the scene
   */
  struct geometry_t {
    // information relevant for the build, about the primtivies in the scene
    std::vector<primitive_t>& primitives;
    // indices into the primitives vector
    uint32_t start, end;
    // the bounding volume for this subset of the primitives in the scene
    aabb_t bounds;
    aabb_t centroid_bounds;

    inline geometry_t(const geometry_t& parent)
      : primitives(parent.primitives)
      , start(0)
      , end(0)
    {}

    inline geometry_t(
        std::vector<primitive_t>& primitives
      , uint32_t start
      , uint32_t end)
      : primitives(primitives), start(start), end(end)
    {
      for (auto i=start; i<end; ++i) {
	bounds = bounds::merge(bounds, primitives[i].bounds);
	centroid_bounds = bounds::merge(centroid_bounds, primitives[i].centroid);
      }
    }

    inline geometry_t& operator=(const geometry_t& g) {
      primitives      = g.primitives;
      start           = g.start;
      end             = g.end;
      bounds          = g.bounds;
      centroid_bounds = g.centroid_bounds;
      return *this;
    }

    inline uint32_t count() const {
      return end - start;
    }

    inline const primitive_t& primitive(uint32_t i) const {
      return primitives[start+i];
    }

    template<typename F>
    inline void partition(const F& f, geometry_t& l, geometry_t& r) {
      auto p   = std::partition(&primitives[start], &primitives[end-1]+1, f);
      auto mid = (uint32_t) (p - &primitives[0]);

      l = {primitives, start, mid};
      r = {primitives, mid, end};
    }
  };

  struct split_t {
    uint32_t axis;
    uint32_t bin;
    float    cost;

    inline split_t(uint32_t axis, uint32_t bin, float cost)
      : axis(axis), bin(bin), cost(cost)
    {}
  };

  struct bin_t {
    uint32_t count;
    aabb_t   bounds;

    inline bin_t()
      : count(0)
    {}

    inline void add(const primitive_t& p) {
      bounds = bounds::merge(bounds, p.bounds);
      count++;
    }
  };

  template<int N>
  struct bins_t {
    bin_t bins[N];

    inline const bin_t& operator[](uint32_t i) const {
      return bins[i];
    }

    inline void add(const aabb_t& bounds, const primitive_t& p, uint8_t axis) {
      bins[find(bounds, p, axis)].add(p);
    }

    static inline uint32_t find(const aabb_t& bounds, const primitive_t& p, uint8_t axis) {
      auto offset = bounds::offset(bounds, p.centroid).v[axis];
      return std::min((int) (N * offset), N-1);
    }
  };

  struct node_t {
    geometry_t primitives; // the  primitives in this node
  };

  inline bool too_large(const geometry_t& g) {
    return g.count() > MAX_PRIMS_IN_NODE;
  }

  inline float leaf_cost(const split_t& s, const geometry_t& g) {
    return g.count();
  }

  split_t find(const geometry_t& geometry) {
    auto best_axis = 0;
    auto best_bin  = 0;
    auto best_cost = std::numeric_limits<float_t>::max();

    for (auto axis=0; axis<3; ++axis) {
      bins_t<NUM_SPLIT_BINS> bins;

      if (geometry.centroid_bounds.empty_on(axis)) {
	continue;
      }

      for (auto i=0; i<geometry.count(); ++i) {
	bins.add(geometry.centroid_bounds, geometry.primitive(i), axis);
      }

      auto split_cost = std::numeric_limits<float_t>::max();
      auto split_bin  = 0;

      for (auto i=0; i<NUM_SPLIT_BINS-1; ++i) {
	aabb_t a, b;
	auto left = 0; auto right = 0;
	for (auto j=0; j<=i; ++j) {
	  a = bounds::merge(a, bins[j].bounds);
	  left += bins[j].count;
	}

	for (auto j=i+1; j<NUM_SPLIT_BINS; ++j) {
	  b = bounds::merge(b, bins[j].bounds);
	  right += bins[j].count;
	}

	auto cost = (left * a.area() + right * b.area()) / geometry.bounds.area();
	if (cost < split_cost) {
	  split_cost = cost;
	  split_bin = i;
	}
      }

      if (split_cost < best_cost) {
	best_axis = axis;
	best_cost = split_cost;
	best_bin  = split_bin;
      }
    }
    return split_t(best_axis, best_bin, best_cost);
  }

  void split(const split_t& split, geometry_t& parent, geometry_t& l, geometry_t& r) {
    parent.partition([&](const primitive_t& p) {
	auto bin = bins_t<NUM_SPLIT_BINS>::find(parent.centroid_bounds, p, split.axis);
	return bin <= split.bin; 
      }, l, r);
  }

  int32_t largest_node(const geometry_t* node, uint32_t n) {
    int32_t out = -1;
    float   a   = std::numeric_limits<float>::max(); 
    for (auto i=0; i<n; ++i) {
      if (node[i].count() < MAX_PRIMS_IN_NODE) {
	continue;
      }

      auto node_area = node[i].bounds.area();
      if (node_area < a) {
	out = i;
	a   = node_area;
      }
    }
    return out;
  }

  template<typename Things, typename BVH>
  uint32_t from(geometry_t& geometry, const Things& things, BVH& bvh) {
    auto n = geometry.count();
    auto s = find(geometry);

    if (geometry.count() <= 8 && leaf_cost(s, geometry) <= 1.0f + s.cost) {
      return 0;
    }

    auto num_children = 2;
    geometry_t children[8] = { [0 ... 7] = { geometry } };

    split(s, geometry, children[0], children[1]);

    while (num_children < 8) {
      auto split_child = largest_node(children, num_children);
      if (split_child == -1) {
	break;
      }
      auto s = find(children[split_child]);
      // check sha heuristic?
      geometry_t tmp(geometry);
      split(s, children[split_child], tmp, children[num_children]);
      children[split_child] = tmp;

      ++num_children;
    }

    // make a new node in the BVH
    auto node_index = bvh.make_node();

    int32_t child_indices[num_children];
    for (int i=0; i<num_children; ++i) {
      child_indices[i] = from(children[i], things, bvh);
    }

    auto node = bvh.resolve(node_index);
    for (int i=0; i<num_children; ++i) {
      node->set_bounds(i, children[i].bounds);

      if (child_indices[i]) {
	node->offset[i] = child_indices[i];
      }
      else {
	auto index =
	  bvh.insert_things(
	    children[i].start, children[i].end,
	    geometry.primitives,
	    things);

	node->offset[i] = index;
	node->num[i]    = children[i].count();
      }
    }

    return node_index;
  }
}
