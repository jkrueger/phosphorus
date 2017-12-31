#pragma once

#include "util/color.hpp"
#include "math/ray.hpp"
#include "thing.hpp"

#include <memory>
#include <string>

struct scene_t;

struct film_t {

  uint32_t width;
  uint32_t height;

  color_t* samples;
  
  inline film_t(uint32_t w, uint32_t h)
    : width(w), height(h) {
    samples = new color_t[w*h];
  }

  inline void expose(uint32_t x, uint32_t y, const color_t& c) {
    samples[y*width+x] = c;
  }

  inline const color_t& pixel(uint32_t x, uint32_t y) const {
    return samples[y*width+x];
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
    double ratio = (double) film.width / (double) film.height;
    double stepx = 1.0/film.width;
    double stepy = 1.0/film.height;
    for (auto y=0; y<film.height; ++y) {
      for (auto x=0; x<film.width; ++x) {
	auto ndcx = (-0.5 + x * stepx) * ratio;
	auto ndcy = 0.5 - y * stepy;

	auto ray = lens.sample(ray_t({0, 0, -10}, {ndcx, ndcy, 1.0}));
	ray.direction.normalize();

	film.expose(x, y, integrator.trace(scene, ray));
      }
    }
  }
};
