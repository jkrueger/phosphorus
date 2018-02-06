#pragma once

#include "integrator/direct.hpp"
#include "precision.hpp"
#include "math/orthogonal_base.hpp"
#include "math/ray.hpp"
#include "shading.hpp"
#include "thing.hpp"
#include "util/algo.hpp"
#include "util/color.hpp"
#include "util/stats.hpp"

#include <algorithm>
#include <atomic>
#include <memory>
#include <string>
#include <thread>

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
  direct_t   direct;

  stats_t::p stats;

  inline camera_t(const vector_t& p, const vector_t& d, const vector_t& up, stats_t::p& stats)
    : position(p), b(d, up), integrator(3, 8), direct(3), stats(stats)
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
    typedef typename Film::splat_t splat_t;

    sample_t* samples = new sample_t[film.samples];
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
	uint32_t num_splats = square(Film::PATCH_SIZE) * film.samples;

	char* buffer = new char[sizeof(shading_info_t) * num_splats];
	
	typename Film::patch_t patch;
        while (film.next_patch(patch)) {
	  auto ray = 0;

	  ray_t    primary[num_splats];
	  splat_t  splats[num_splats];

	  shading_info_t* info = new(buffer) shading_info_t[num_splats];
	  
	  for (auto y=patch.y; y<patch.yend(); ++y) {
	    for (auto x=patch.x; x<patch.xend(); ++x) {
	      auto ndcx = (-0.5f + x * stepx) * ratio;
	      auto ndcy = 0.5f - y * stepy;

	      for (int i=0; i<film.samples; ++i, ++ray) {
		auto sx = samples[i].u - 0.5f;
		auto sy = samples[i].v - 0.5f;

		primary[ray].origin = position;
		primary[ray].direction =
		  b.to_world({
		    sx * stepx + ndcx,
		    sy * stepy + ndcy,
		    1.0f
		  }).normalize();
	      }
	    }
          }

	  for (auto i=0; i<num_splats; ++i) {
	    // TODO: sort hits by mesh for deferred shading
	    scene.intersect(primary[i], info[i]);
	  }

	  for (auto i=0; i<num_splats; ++i) {
	    color_t c;

	    if (info[i].d < std::numeric_limits<float>::max()) {
	      c = direct.li(scene, (position - info[i].p).normalize(), info[i]) +
		integrator.li(scene, primary[i], info[i]);
	    }

	    splats[i].x = samples[i % film.samples].u - 0.5f;
	    splats[i].y = samples[i % film.samples].v - 0.5f;
	    splats[i].c = c;
	  }

	  film.apply_splats(patch, splats);

	  // iterate over meshes
	  //   for all hits on mesh
	  //     convert light samples to shadow rays
	  //     trace rays
	  //     evaluate brdf for all hits
	  //   convert hits to brdf samples
	  // convert all brdf samples to path segment rays
	  // trace all rays
	  // repeat
	  // write splats to film
	}
      });
    }

    for (int t=0; t<cores; ++t) {
      if (threads[t].joinable()) {
	threads[t].join();
      }
    }
  }
};
