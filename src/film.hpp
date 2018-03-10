#pragma once

#include "util/algo.hpp"

#include <algorithm>

struct film_t {
  typedef std::shared_ptr<film_t> p;

  static const uint32_t PATCH_SIZE;

  struct pixel_t {
    color_t  c;
    uint32_t padding;
  };

  struct patch_t {
    uint32_t x, y, w, h;
  };

  struct splat_t {
    color_t  c;
    uint32_t padding;
  };

  struct samples_t {
    float_t* x;
    float_t* y;

    inline samples_t(allocator_t& a, uint32_t num) {
      x = new(a) float_t[num];
      y = new(a) float_t[num];
    }
  };

  uint32_t width;
  uint32_t height;
  float_t  ratio;
  float_t  stepx;
  float_t  stepy;
  uint32_t spp;
  uint32_t spd;
  uint32_t num_samples;
  uint32_t num_patches;

  pixel_t*  pixels;
  sample_t* stratified_pattern;

  std::atomic_int patch;

  inline film_t(uint32_t w, uint32_t h, uint32_t spd)
    : width(w)
    , height(h)
    , spd(spd)
    , spp(spd*spd)
    , patch(0)
  {
    num_samples = w*h*spp;
    num_patches = (w/PATCH_SIZE)*(h/PATCH_SIZE);

    ratio = (float_t) width / (float_t) height;
    stepx = 1.0f/width;
    stepy = 1.0f/height;

    stratified_pattern = new sample_t[spp];
    if (spp == 1) {
      stratified_pattern[0].u = 0;
      stratified_pattern[0].v = 0;
    }
    else {
      sampling::strategies::stratified_2d(stratified_pattern, spd);
    }

    // allocate a single frame buffer for the output
    pixels = new pixel_t[w*h];
  }

  inline bool next_patch(patch_t& out) {
    auto p = patch++;
    if (p < num_patches) {
      out.x = (p * PATCH_SIZE) % width;
      out.y = ((p * PATCH_SIZE) / width) * PATCH_SIZE;
      out.w = PATCH_SIZE;
      out.h = PATCH_SIZE;

      return true;
    }
    return false;
  }

  inline void sample_film(const patch_t& patch, samples_t& out) const {
    auto j=0;
    for (auto y=patch.y; y<patch.y+patch.h; ++y) {
      const auto ndcy = 0.5f - y * stepy;

      for (auto x=patch.x; x<patch.x+patch.w; ++x) {
	const auto ndcx = (-0.5f + x * stepx) * ratio;

	for (auto i=0; i<spp; ++i, ++j) {
	  const auto sx = stratified_pattern[i].u - 0.5f;
	  const auto sy = stratified_pattern[i].v - 0.5f;

	  out.x[j]  = ndcx + stepx * sx;
	  out.y[j]  = ndcy + stepy * sy;
	}
      }
    }
  }

  inline void filter(color_t& c, const splat_t& splat) const {
    c += splat.c;
    /* 
       (std::max(0.0f, 2.0f - std::abs(splat.x)) *
        std::max(0.0f, 2.0f - std::abs(splat.y)));
    */
  }

  inline uint32_t num_splats() const {
    return square(PATCH_SIZE) * spp;
  }

  inline void apply_splats(
    const patch_t& patch
  , const samples_t& samples
  , splat_t* const splats)
  {
    auto s = splats;
    for (auto y=patch.y; y<patch.y+patch.h; ++y) {
      for (auto x=patch.x; x<patch.x+patch.w; ++x) {
	auto pixel = y*width+x;

	for (auto i=0; i<spp; ++i, ++s) {
	  color_t c;
	  filter(c, *s);

	  pixels[pixel].c += c.scale(1.0f/spp);
	}
      }
    }
  }

  inline const color_t pixel(uint32_t x, uint32_t y) const {
    const auto& p = pixels[y*width+x];
    return p.c;
  }
};
