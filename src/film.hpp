#pragma once

#include "util/algo.hpp"

#include <algorithm>

struct film_t {
  typedef std::shared_ptr<film_t> p;

  static const uint32_t SAMPLES_PER_PATCH = 256;

  struct pixel_t {
    color_t c;
  };

  struct splat_t {
    float    x, y;
    color_t  c;
    uint32_t padding;
  };

  struct samples_t {
    float x[SAMPLES_PER_PATCH];
    float y[SAMPLES_PER_PATCH];
  };

  uint32_t  width;
  uint32_t  height;
  uint32_t  spd;
  uint32_t  num_samples;
  uint32_t  num_patches;
  uint32_t  pixels_per_path;
  uint32_t  ppd;
  uint32_t  patches_per_pixel;

  pixel_t*  pixels;
  sample_t* stratified_pattern;

  std::atomic_int patch;

  inline film_t(uint32_t w, uint32_t h, uint32_t spd)
    : width(w)
    , height(h)
    , spd(spd)
    , samples(spd*spd)
  {
    num_samples       = w*h*samples;
    num_patches       = num_samples / SAMPLES_PER_PATCH;
    pixels_per_patch  = num_samples / num_patches;
    ppd               = (uint32_t) std::sqrt(ppp);
    patches_per_pixel = SAMPLES_PER_PATCH / samples;

    stratified_pattern = new sample_t[samples];
    if (samples == 1) {
      samples[0].u = 0;
      samples[0].v = 0;
    }
    else {
      sampling::strategies::stratified_2d(samples, spd);
    }

    // allocate a single frame buffer for the output
    pixels = new pixel_t[w*h];
  }

  inline bool sample_film(samples_t& out) {
    auto patch = patch++;
    if (patch < num_patches) {
      auto q  = (patch % patches_per_pixel);
      auto p  = patch - q;
      auto xs = (p * ppd) % width;
      auto ys = ((p * ppd) / width) * ppd;

      for (auto y=ys; y<ys+ppd; ++y) {
	for (auto x=xs; x<xs+ppd; ++x) {
	  
	}
      }

      return true;
    }
    return false;
  }

  inline void filter(color_t& c, const splat_t& splat) const {
    c += splat.c;
    /* 
       (std::max(0.0f, 2.0f - std::abs(splat.x)) *
        std::max(0.0f, 2.0f - std::abs(splat.y)));
    */
  }

  inline uint32_t num_splats() const {
    return SAMPLES_PER_PATCH;
  }

  inline void apply_splats(const patch_t& patch, splat_t* const splats) {
    splat_t* s = splats;
    for (auto y=0; y<patch.height; ++y) {
      for (auto x=0; x<patch.width; ++x) {
	auto pixel = (patch.y + y) * width + (patch.x + x);

	color_t  c;
	splat_t* end = s + samples;
	while (s != end) {
	  filter(c, *s);
	  ++s;
	}
	c.scale(1.0f/samples);

	pixels[pixel].c += c;
      }
    }
  }

  inline const color_t pixel(uint32_t x, uint32_t y) const {
    const auto& p = pixels[y*width+x];
    return p.c;
  }
};
