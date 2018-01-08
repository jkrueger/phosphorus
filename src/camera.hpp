#pragma once

#include "util/color.hpp"
#include "math/ray.hpp"
#include "thing.hpp"

#include <algorithm>
#include <atomic>
#include <memory>
#include <string>
#include <thread>

struct scene_t;

struct film_t {

  uint32_t width;
  uint32_t height;
  uint32_t samples;

  struct pixel_t {
    uint16_t n;
    color_t  c;

    pixel_t() : n(0) {}
  };

  struct sample_t {
    double x, y;
    color_t c;
  };

  struct area_t {
    uint32_t x, y, width, height;
    sample_t* samples;
  };

  pixel_t* pixels;
  area_t*  areas;

  std::atomic_int area;

  inline film_t(uint32_t w, uint32_t h, uint32_t s)
    : width(w), height(h), samples(s), area((w/128) * (h/128)) {
    pixels = new pixel_t[w*h];
    areas  = new area_t[(w/128)*(h/128)];

    uint32_t hareas = width/128;

    for (uint32_t y=0; y<height/128; ++y) {
      for (uint32_t x=0; x<hareas; ++x) {
	area_t& area = areas[y * hareas + x];
	area.x       = 128 * x;
	area.y       = 128 * y;
	area.width   = 128;
	area.height  = 128;
	area.samples = new sample_t[128*128*samples];
      }
    }
  }

  inline area_t* next_area() {
    auto idx = area--;
    if (idx >= 0) {
      printf("progress: %d\n", idx);
      return &areas[idx];
    }
    return nullptr;
  }

  inline void finalize() {
    printf("finalize\n");
    for (int i=0; i<(width/128)*(height/128); ++i) {
      auto& area = areas[i];
      for (int y=0; y<area.height; ++y) {
	for (int x=0; x<area.width; ++x) {
	  uint32_t index = (y*area.width*samples) + (x*samples);
	  for (int s=0; s<samples; ++s) {
	    auto& sample = area.samples[index + s];
	    auto& pixel  = pixels[(area.y + y) * width + (area.x + x)];

	    pixel.c += sample.c *
	      (std::max(0.0, 2.0 - std::abs(sample.x)) *
	       std::max(0.0, 2.0 - std::abs(sample.y)));
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

  vector_t p;
  vector_t d;
  vector_t up;
  vector_t left;

  Integrator integrator;

  inline camera_t(vector_t _p, vector_t _d, vector_t _up)
    : p(_p), d(_d), up(_up), left(cross(d, up))
  {}

  template<typename Film, typename Lens>
  void snapshot(Film& film, const Lens& lens, const thing_t& scene) {

    auto samples_per_dimension = (uint32_t) std::sqrt(film.samples);

    sample_t pixels[film.samples];
    sampling::strategies::stratified_2d(pixels, (uint32_t) std::sqrt(film.samples));

    double ratio = (double) film.width / (double) film.height;
    double stepx = 1.0/film.width;
    double stepy = 1.0/film.height;

    std::thread threads[4];

    for (auto t=0; t<4; ++t) {
      threads[t] = std::thread([&]() {
	  while (auto area=film.next_area()) {
	    auto n    = 0;
	    auto xend = area->x + area->width;
	    auto yend = area->y + area->height;
	    for (auto y=area->y; y<yend; ++y) {
	      for (auto x=area->x; x<xend; ++x) {
		auto ndcx = (-0.5 + x * stepx) * ratio;
		auto ndcy = 0.5 - y * stepy;

		for (int i=0; i<film.samples; ++i) {
		  auto sx  = pixels[i].u - 0.5;
		  auto sy  = pixels[i].v - 0.5;
		  ray_t ray({0, 0, -10}, {sx * stepx + ndcx, sy * stepy + ndcy, 1.0});
		  ray.direction.normalize();

		  auto &sample = area->samples[n++];
		  sample.c = integrator.trace(scene, ray);
		  sample.x = sx;
		  sample.y = sy;
		}
	      }
	    }
	  }
	});
    }

    for (int t=0; t<4; ++t) {
      threads[t].join();
    }

    film.finalize();
  }
};
