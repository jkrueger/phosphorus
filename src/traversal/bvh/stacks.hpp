#pragma once

struct node_ref_t {
  uint32_t offset : 28;
  uint32_t flags  : 4;
  float    d;
};

namespace stream {
  static const uint32_t RAYS_PER_LANE = (2<<17)-1;
  
  struct lanes_t {
    uint32_t active[8][RAYS_PER_LANE];
    uint32_t num[8];

    lanes_t() {
      memset(num, 0, sizeof(uint32_t) * 8);
    }
  };

  struct task_t {
    uint32_t offset;
    uint32_t num_rays;
    uint32_t lane: 8, flags: 8, prims: 16;
    uint32_t padding;

    inline bool is_leaf() const {
      return flags == 1;
    }
  };
}

template<typename Node>
inline void push(
  node_ref_t* stack
, int32_t& top
, const float* const dists
, const Node* node
, uint32_t idx)
{
  stack[top].offset = node->offset[idx];
  stack[top].flags  = node->num[idx];
  stack[top].d      = dists[idx];
  ++top;
}

inline void push(
  stream::task_t* stack
, int32_t& top
, uint16_t num)
{
  stack[top].offset   = 0;
  stack[top].num_rays = num;
  stack[top].lane     = 0;
  stack[top].flags    = 0;
  stack[top].prims    = 0;

  ++top;
}

template<typename Node>
inline void push(
  stream::task_t* stack
, int32_t& top
, const Node* node
, uint8_t  lane
, uint16_t num)
{
  stack[top].offset   = node->offset[lane];
  stack[top].num_rays = num;
  stack[top].lane     = lane;
  stack[top].flags    = node->flags[lane];
  stack[top].prims    = node->num[lane];

  ++top;
}

inline void push(stream::lanes_t& lanes, uint8_t lane, uint32_t id) {
  auto& a = lanes.num[lane];
  lanes.active[lane][a++] = id;
}

inline uint32_t* pop(stream::lanes_t& lanes, uint8_t lane, uint32_t num) {
  auto& a = lanes.num[lane];
  a -= num;

  return &(lanes.active[lane][a]);
}
