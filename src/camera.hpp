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

template<typename Integrator, typename Film, typename Lens>
struct camera_t {
  typedef std::shared_ptr<camera_t> p;

  vector_t          position;
  orthogonal_base_t orientation;

  Integrator integrator;
  Film       film;
  Lens       lens;

  stats_t::p stats;

  inline camera_t(const vector_t& p, const vector_t& d, const vector_t& up, stats_t::p& stats)
    : position(p), orientation(d, up), integrator(8), stats(stats)
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
    const auto ratio = (float_t) film.width / (float_t) film.height;
    const auto stepx = 1.0f/film.width;
    const auto stepy = 1.0f/film.height;

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

  inline void reset_deferred_buffers(by_material_t* deferred) {
    for (auto i=0; i<material_t::ids; ++i) {
      deferred[i].material = scene.materials[i];
      deferred[i].splats.num = 0;
    }
  }

  inline void find_next_path_vertices(segment_t* segments, active_t& active) {
    // find intersection points following path vertices
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
  }

  inline void sample_direct_lighting(
    const bxdf_t::p& bxdf
  , segment_t* segments
  , const active_t& deferred
  , splat_t* splats)
  {
    integrator.sample_lights(segments, deferred);
    integrator.shade(bxdf, deferred, splats);
  }

  inline void sample_path_directions(
    const bxdf_t::p& bxdf
  , segment_t* segments
  , const active_t& deferred
  , const active_t& active)
  {
    integrator.sample_path_directions(bxdf, segments, deferred, active);
  }

  template<typename Scene>
  void snapshot(const Scene& scene) {
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
	  // allocate patch buffers
	  segment_t*     segments = new(allocator) segment_t[num_splats];
	  by_material_t* deferred = new(allocator) by_material_t[material_t::ids];
	  splat_t*       splats   = new(allocator) splat_t[num_splats];

	  // sample all rays for this patch
	  sample_camera_vertices(patch, segments, active, num_splats);

	  // TODO: find first hit separately and compute direct light contribution
	  // with stratified samples?

	  // run rendering pipeline for patch 
	  while (has_live_paths(active)) {
	    reset_deferred_buffers(deferred);
	    find_next_path_vertices(segments, active, deferred);

	    auto material = deferred;
	    for (auto i=0; i<material_t::ids; ++i, ++material) {
	      if (material->splats.num > 0) {
		auto bxdf = material->material->at(allocator);
		sample_direct_lighting(bxdf, segments, material->splats, splats);
		sample_path_directions(bxdf, segments, active, material->splats);
	      }
	    }
	  }

	  film.apply_splats(patch, splats);
	  stats->areas++;

	  // free all memory allocated while rendering this patch, without
	  // calling any destructors
	  allocator.reset();
      });
    }

    for (int t=0; t<cores; ++t) {
      if (threads[t].joinable()) {
	threads[t].join();
      }
    }
  }
};
