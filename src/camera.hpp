#pragma once

#include "util/color.hpp"
#include "math/ray.hpp"
#include "thing.hpp"

#include <algorithm>
#include <memory>
#include <string>

struct scene_t;

struct film_t {

  uint32_t width;
  uint32_t height;

  struct pixel_t {
    uint8_t n;
    color_t c;

    pixel_t() : n(0) {}
  };

  pixel_t* samples;

  inline film_t(uint32_t w, uint32_t h)
    : width(w), height(h) {
    samples = new pixel_t[w*h];
  }

  inline void expose(uint32_t x, uint32_t y, double sx, double sy, const color_t& c) {
    uint32_t index = y*width+x;
    samples[index].c += c *
      (std::max(0.0, 2.0 - std::abs(sx)) *
       std::max(0.0, 2.0 - std::abs(sy)));
    samples[index].n++;
  }

  inline const color_t pixel(uint32_t x, uint32_t y) const {
    const auto& sample = samples[y*width+x];
    return sample.c * (1.0/sample.n);
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

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0,0.125);

    double ratio = (double) film.width / (double) film.height;
    double stepx = 1.0/film.width;
    double stepy = 1.0/film.height;
    for (auto y=0; y<film.height; ++y) {
      for (auto x=0; x<film.width; ++x) {
	auto ndcx = (-0.5 + x * stepx) * ratio;
	auto ndcy = 0.5 - y * stepy;

	double sstep = 0.125;
	for (int i=0; i<8; ++i) {
	  for (int j=0; j<8; ++j) {
	    auto sx  = i * sstep + dis(gen);
	    auto sy  = j * sstep + dis(gen);
	    auto ray = lens.sample(ray_t({0, 0, -10}, {sx * stepx + ndcx, sy * stepy + ndcy, 1.0}));
	    ray.direction.normalize();
	    auto c   = integrator.trace(scene, ray);
	    film.expose(x, y, sx - 0.5, sy - 0.5, c);
	  }
	}
      }

      printf("\rprogress: %d", (uint32_t) (((double)y/(double)film.height) * 100.0));
      fflush(stdout);
    }
  }
};
