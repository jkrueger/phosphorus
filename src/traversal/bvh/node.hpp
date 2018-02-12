#pragma once

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

    for (int i=0; i<N*3; ++i) {
      bounds[i]       = std::numeric_limits<float>::max();
      bounds[i+(N*3)] = std::numeric_limits<float>::lowest();
    }

    memset(offset, 0, N*sizeof(uint32_t));
    memset(num, 0, N*sizeof(uint8_t));
  }

  inline void set_bounds(uint32_t i, const aabb_t& b) {
    bounds[i     ] = b.min.x;
    bounds[i +  8] = b.min.y;
    bounds[i + 16] = b.min.z;
    bounds[i + 24] = b.max.x;
    bounds[i + 32] = b.max.y;
    bounds[i + 40] = b.max.z;
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
typedef mbvh_node_t<8> octa_node_t;
