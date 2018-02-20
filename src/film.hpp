#pragma once

#include "util/algo.hpp"

#include <algorithm>

struct film_t {
  typedef std::shared_ptr<film_t> p;

  static const uint32_t SAMPLES_PER_PATCH;

  struct pixel_t {
    color_t c;
  };

  struct splat_t {
    float    x, y;
    color_t  c;
    uint32_t padding;
  };

  struct patch_t {
    uint32_t x, y, width, height;

    inline uint32_t xend() const {
      return x+width;
    }

    inline uint32_t yend() const {
      return y+height;
    }
  };
  
  uint32_t width;
  uint32_t height;
  uint32_t spd;
  uint32_t samples;
  uint32_t pixels_per_patch;
  uint32_t ppd;
  uint32_t num_patches;

  pixel_t* pixels;

  std::atomic_int patch;

  inline film_t(uint32_t w, uint32_t h, uint32_t spd)
    : width(w)
    , height(h)
    , spd(spd)
    , samples(spd*spd)
    , patch(0)
  {
    num_patches      = (w*h*samples) / SAMPLES_PER_PATCH;
    pixels_per_patch = std::max(w*h / num_patches, 1u);
    ppd              = (uint32_t) std::sqrt(pixels_per_patch);

    // allocate a single frame buffer for the output
    pixels = new pixel_t[w*h];
  }

  inline bool next_patch(patch_t& out) {
    auto p = patch++;
    if (p < num_patches) {
      out.x      = (p * ppd) % width;
      out.y      = ((p * ppd) / width) * ppd;
      out.width  = ppd;
      out.height = ppd;
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
	splat_t* end = s + SAMPLES_PER_PATCH;
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
