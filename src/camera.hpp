#pragma once

#include "precision.hpp"
#include "math/orthogonal_base.hpp"
#include "math/ray.hpp"
#include "thing.hpp"
#include "util/color.hpp"
#include "util/stats.hpp"

#include <algorithm>
#include <atomic>
#include <memory>
#include <string>
#include <thread>

struct film_t {

  uint32_t width;
  uint32_t height;
  uint32_t spd;
  uint32_t samples;

  struct pixel_t {
    uint16_t n;
    color_t  c;

    pixel_t() : n(0) {}
  };

  struct sample_t {
    float_t x, y;
    color_t c;
  };

  struct area_t {
    uint32_t x, y, width, height;
    sample_t* samples;
  };

  pixel_t* pixels;
  area_t*  areas;

  uint32_t        num_areas;
  std::atomic_int area;

  inline film_t(uint32_t w, uint32_t h, uint32_t spd)
    : width(w), height(h), spd(spd), samples(spd*spd),
      num_areas((w/16) * (h/16)),
      area(num_areas) {
    pixels = new pixel_t[w*h];
    areas  = new area_t[(w/16)*(h/16)];

    uint32_t hareas = width/16;

    for (uint32_t y=0; y<height/16; ++y) {
      for (uint32_t x=0; x<hareas; ++x) {
	area_t& area = areas[y * hareas + x];
	area.x       = 16 * x;
	area.y       = 16 * y;
	area.width   = 16;
	area.height  = 16;
	area.samples = new sample_t[16*16*samples];
      }
    }
  }

  inline area_t* next_area() {
    auto idx = area--;
    if (idx >= 0) {
      return &areas[idx];
    }
    return nullptr;
  }

  inline void finalize() {
    for (int i=0; i<(width/16)*(height/16); ++i) {
      auto& area = areas[i];
      for (int y=0; y<area.height; ++y) {
	for (int x=0; x<area.width; ++x) {
	  uint32_t index = (y*area.width*samples) + (x*samples);
	  for (int s=0; s<samples; ++s) {
	    auto& sample = area.samples[index + s];
	    auto& pixel  = pixels[(area.y + y) * width + (area.x + x)];

	    pixel.c += sample.c *
	      (std::max(0.0f, 2.0f - std::abs(sample.x)) *
	       std::max(0.0f, 2.0f - std::abs(sample.y)));
	    pixel.n++;
	  }
	}
      }
    }
  }

  inline const color_t pixel(uint32_t x, uint32_t y) const {
    const auto& p = pixels[y*width+x];
    return p.c * (1.0/p.n);
  }
};

namespace lenses {

  struct pinhole_t {

    ray_t sample(const ray_t& ray) const {
      return ray;
    }
  };
}

template<typename Integrator>
struct camera_t {
  typedef std::shared_ptr<camera_t> p;
  
  vector_t        position;
  orthogonal_base b;

  Integrator integrator;

  sample_t* samples;

  stats_t::p stats;

  inline camera_t(const vector_t& p, const vector_t& d, const vector_t& up, stats_t::p& stats)
    : position(p), b(d, up), integrator(3, 8), stats(stats)
  {}

  static inline p look_at(
    stats_t::p& stats,
    const vector_t& pos, const vector_t& at,
    const vector_t& up = vector_t(0.0, 1.0, 0.0)) {

    auto z = normalize(at - pos);
    auto x = normalize(cross(z, up));
    
    return p(new camera_t(pos, z, up, stats));
  }

  template<typename Film, typename Lens, typename Scene>
  void snapshot(Film& film, const Lens& lens, const Scene& scene) {

    samples = new sample_t[film.samples];
    sampling::strategies::stratified_2d(samples, film.spd);

    float_t ratio = (float_t) film.width / (float_t) film.height;
    float_t stepx = 1.0/film.width;
    float_t stepy = 1.0/film.height;

    // automatically use all cores for now
    uint32_t cores = std::thread::hardware_concurrency();
    printf("Using %d threads for rendering\n", cores);

    std::thread threads[cores];

    for (auto t=0; t<cores; ++t) {
      threads[t] = std::thread([&]() {
	  while (auto area=film.next_area()) {
	    auto n    = 0;
	    auto xend = area->x + area->width;
	    auto yend = area->y + area->height;
	    for (auto y=area->y; y<yend; ++y) {
	      for (auto x=area->x; x<xend; ++x) {
		auto ndcx = (-0.5f + x * stepx) * ratio;
		auto ndcy = 0.5f - y * stepy;

		for (int i=0; i<film.samples; ++i) {
		  auto sx  = samples[i].u - 0.5f;
		  auto sy  = samples[i].v - 0.5f;

		  const auto dir = b.to_world({
		    sx * stepx + ndcx, sy * stepy + ndcy, 1.0f
		  }).normalize();

		  const ray_t ray(position, dir);

		  auto &sample = area->samples[n++];
		  sample.c = integrator.li(scene, ray);
		  sample.x = sx;
		  sample.y = sy;
		}
	      }
	    }
	    stats->areas++;
	  }
	});
    }

    for (int t=0; t<cores; ++t) {
      if (threads[t].joinable()) {
	threads[t].join();
      }
    }

    film.finalize();
  }
};
