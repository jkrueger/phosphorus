#pragma once

#include "things/mesh.hpp"
#include "precision.hpp"
#include "math/orthogonal_base.hpp"
#include "math/ray.hpp"
#include "shading.hpp"
#include "thing.hpp"
#include "util/algo.hpp"
#include "util/allocator.hpp"
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

  vector_t          position;
  orthogonal_base_t b;

  Integrator integrator;

  stats_t::p stats;

  inline camera_t(const vector_t& p, const vector_t& d, const vector_t& up, stats_t::p& stats)
    : position(p), b(d, up), integrator(8), stats(stats)
  {}

  static inline p look_at(
    stats_t::p& stats,
    const vector_t& pos, const vector_t& at,
    const vector_t& up = vector_t(0.0, 1.0, 0.0)) {

    auto z = normalize(at - pos);
    auto x = normalize(cross(z, up));
    
    return p(new camera_t(pos, z, up, stats));
  }

  template<typename Patch>
  inline void sample_patch(
      const Patch& patch
    , segment_t* segments
    , active_t& active
    , uint32_t num_splats) const
  {
    active.num = num_splats;

    auto splat   = 0;
    auto segment = segments;
    for (auto y=patch.y; y<patch.yend(); ++y) {
      for (auto x=patch.x; x<patch.xend(); ++x) {
	auto ndcx = (-0.5f + x * stepx) * ratio;
	auto ndcy = 0.5f - y * stepy;

	auto segment_end = segment + film.samples;
	do {
	  auto sx = samples[i].u - 0.5f;
	  auto sy = samples[i].v - 0.5f;

	  segment->p  = position;
	  segment->wi = b.to_world({
	      sx * stepx + ndcx
	    , sy * stepy + ndcy
	    , 1.0f
	    }).normalize();

	  active.segment[splat] = splat++;

	} while(++segment != segment_end);
      }
    }
  }

  template<typename Film, typename Lens, typename Scene>
  void snapshot(Film& film, const Lens& lens, const Scene& scene) {
    typedef typename Film::splat_t splat_t;
    typedef typename Film::patch_t patch_t;

    sample_t* samples = new sample_t[film.samples];
    if (film.samples == 1) {
      samples[0].u = 0;
      samples[0].v = 0;
    }
    else {
      sampling::strategies::stratified_2d(samples, film.spd);
    }

    const auto num_splats = film.num_splats();
    const auto ratio = (float_t) film.width / (float_t) film.height;
    const auto stepx = 1.0f/film.width;
    const auto stepy = 1.0f/film.height;

    // automatically use all cores for now
    uint32_t cores = std::thread::hardware_concurrency();
    printf("Using %d threads for rendering\n", cores);

    std::thread threads[cores];

    for (auto t=0; t<cores; ++t) {
      threads[t] = std::thread([&]() {
	allocator_t allocator(1024*1024*100);

	patch_t  patch;  // the current image patch we're rendering
	active_t active; // active path vertices

        while (film.next_patch(patch)) {
	  // allocate frame data structures
	  segment_t*     segments = new(allocator) segment_t[num_splats];
	  by_material_t* deferred = new(allocator) by_material_t[material_t::ids];
	  splat_t*       splats   = new(allocator) splat_t[num_splats];

	  // sample all rays for this patch
	  sample_patch(patch, segments, active, num_splats);

	  // run rendering pipeline for patch 
	  while (active.has_live_paths()) {
	    reset_material_buffers(deferred);
	    find_next_path_vertices(segments, active, deferred);
	    shade_hits();
	    sample_path_directions();
	  }

	  while (active.num > 0) {
	    for (auto i=0; i<material_t::ids; ++i) {
	      deferred[i].splats.num = 0;
	      deferred[i].material   = scene.materials[i];
	    }

	    scene.intersect(segments, active);

	    for (auto i=0; i<active.num; ++i) {
	      auto  index   = active.segment[i];
	      auto& segment = segments[index];
	      if (segment.alive()) {
		auto mesh = scene.meshes[segment.mesh];

		segment.follow();
		segment.n = mesh->shading_normal(segment);

		auto& material = deferred[mesh->material->id];
		material.splats.segment[material.splats.num++] = index;
	      }
	    }

	    active.num = 0;

	    auto material = deferred;
	    for (auto i=0; i<material_t::ids; ++i, ++material) {
	      if (material->splats.num > 0) {
		auto bxdf = material->material->at(allocator);
		integrator.li(scene, segments, material->splats, active, splats, bxdf);
	      }
	    }
	  }

	  film.apply_splats(patch, splats);
	  stats->areas++;

	  // free all memory allocated while rendering this patch, without
	  // calling any destructors
	  allocator.reset();
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
