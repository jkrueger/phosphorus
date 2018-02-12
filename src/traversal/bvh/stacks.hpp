#pragma once

struct node_ref_t {
  uint32_t offset : 28;
  uint32_t flags  : 4;
  float    d;
};

namespace stream {
  static const uint32_t RAYS_PER_LANE = 256;
  
  struct lanes_t {
    uint32_t active[8][RAYS_PER_LANE];
    uint16_t num[8];

    lanes_t() {
      memset(num, 0, sizeof(uint32_t) * 8);
    }
  };

  struct task_t {
    uint32_t offset;
    uint16_t num_rays;
    uint8_t  lane;
    uint8_t  flags;
  };
}

template<typename Node>
inline void push(
  node_ref_t* stack, int32_t& top, const float* const dists,
  const Node* node, uint32_t idx) {
  stack[top].offset = node->offset[idx];
  stack[top].flags  = node->num[idx];
  stack[top].d      = dists[idx];
  ++top;
}

inline void push(
    stream::task_t* stack
  , int32_t& top
  , uint32_t offset
  , uint16_t num
  , uint8_t  lane)
{
  stack[top].offset   = offset;
  stack[top].num_rays = num;
  stack[top].lane     = lane;
  ++top;
}

inline void push(stream::lanes_t& lanes, uint8_t lane, uint32_t id) {
  auto& a = lanes.num[lane];
  lanes.active[lane][a++] = id;
}

inline uint32_t* pop(stream::lanes_t& lanes, uint8_t lane, uint32_t num) {
  auto& a = lanes.num[lane];
  a -= num;

  return &lanes.active[lane][a];
}
