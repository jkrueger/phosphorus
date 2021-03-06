#pragma once

#include "material.hpp"
#include "precision.hpp"
#include "math/ray.hpp"
#include "math/vector.hpp"
#include "util/color.hpp"

#include <limits>

static const uint8_t ALIVE    = 1;
static const uint8_t HIT      = 2;
static const uint8_t MASKED   = 4;

struct segment_t {
  static const bool shade = true;
  static const bool stop_on_first_hit = false;

  vector_t  p; // 12
  vector_t  wi; // 24
  vector_t  n;  // 32
  color_t   beta; // 44
  float_t   d; // 48
  float_t   u; // 52
  float_t   v; // 56
  uint32_t  mesh; // 60
  uint32_t  face; // 64
  uint32_t  flags : 8, depth : 8, material: 16; // 68
  vector_t  wo; // 80
  float_t   s;
  float_t   t;
  // TODO: ray differentials, light contribution
  char     padding[40];

  inline segment_t()
    : beta(1.0f)
    , d(std::numeric_limits<float>::max())
    , flags((uint8_t) ALIVE)
    , depth(0)
  {}

  inline void kill() {
    flags &= ~ALIVE;
  }

  inline void revive() {
    flags |= ALIVE;
  }

  inline void miss() {
    flags &= ~HIT;
  }

  inline void hit() {
    flags |= HIT;
  }

  inline bool is_alive() const {
    return (flags & ALIVE) == ALIVE;
  }

  inline bool is_hit() const {
    return (flags & HIT) == HIT;
  }

  inline constexpr bool masked() const {
    return false;
  }

  inline void follow() {
    p = p + d * wi;
    d = std::numeric_limits<float>::max();
  }

  inline void shading(float u, float v, uint32_t m, uint32_t f) {
    u    = u;
    v    = v;
    mesh = m;
    face = f;
  }
};

struct occlusion_query_t {
  static const bool shade = false;
  static const bool stop_on_first_hit = true;
  
  vector_t p;     // 12
  vector_t wi;    // 24
  float    d;     // 28
  uint32_t flags; // 32
  color_t  e;     // 44
  float    pdf;   // 48

  char padding[16]; // pad to 64 bytes

  occlusion_query_t()
    : d(std::numeric_limits<float>::max())
    , flags(0)
  {}

  inline void mask() {
    flags |= MASKED;
  }

  inline void kill() {
    flags &= ~ALIVE;
  }

  inline void revive() {
    flags |= ALIVE;
  }

  inline void miss() {
    flags &= ~HIT;
  }

  inline void hit() {
    flags |= HIT;
  }

  inline bool is_hit() const {
    return (flags & HIT) == HIT;
  }

  inline bool is_alive() const {
    return (flags & ALIVE) == ALIVE;
  }

  inline bool masked() const {
    return (flags & MASKED) == MASKED;
  }

  inline bool occluded() const {
    return is_hit() | masked();
  }

  inline void shading(float u, float v, uint32_t m, uint32_t f)
  {}
};

struct active_t {
  uint32_t num;
  uint32_t segment[256];

  inline void clear() {
    num = 0;
  }
};

struct by_material_t {
  material_t* material;
  active_t    splats;
};

namespace shading {

  inline bool has_live_paths(const active_t& active) {
    return active.num > 0;
  }

  template<typename T>
  inline void offset(T& t, const vector_t& n) {
    float_t offset = 0.0001f;
    if (!in_same_hemisphere(t.wi, n)) {
      offset = -offset;
    }
    t.p = t.p + n * offset;
  }
}
