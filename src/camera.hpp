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
    typedef std::shared_ptr<pinhole_t> p;

    ray_t sample(const ray_t& ray) const {
      return ray;
    }
  };
}

template<typename Film, typename Lens, typename Integrator>
struct camera_t {
  typedef std::shared_ptr<camera_t> p;

  typedef typename Film::patch_t   patch_t;
  typedef typename Film::splat_t   splat_t;
  typedef typename Film::samples_t samples_t;

  static const uint32_t SAMPLES_PER_ITERATION=256;

  vector_t          position;
  orthogonal_base_t orientation;

  typename Film::p film;
  typename Lens::p lens;

  stats_t::p stats;

  inline camera_t(
    const typename Film::p& film
  , const typename Lens::p& lens
  , stats_t::p& stats)
    : orientation({0,0,1}, {0,1,0})
    , film(film)
    , lens(lens)
    , stats(stats)
  {
  }

  void look_at(
    const vector_t& pos
  , const vector_t& at
  , const vector_t& up = vector_t(0.0, 1.0, 0.0))
  {
    auto z = normalize(at - pos);
    auto x = normalize(cross(z, up));

    position    = pos;
    orientation = orthogonal_base_t(z, up);
  }

  inline void sample_camera_vertices(
    const patch_t& patch
  , samples_t& samples
  , segment_t* segments
  , active_t& active
  , uint32_t num_splats) const
  {
    film->sample_film(patch, samples);

    auto segment = segments;
    for (auto i=0; i<num_splats; ++i, ++segment) {
      segments[i].p  = position;
      segments[i].wi =
      	orientation.to_world({
      	    samples.x[i]
      	  , samples.y[i]
      	  , 1.0f
      	}).normalize();
    }
  }

  inline void activate_samples(active_t& active, uint32_t start, uint32_t num) const {
    for (auto i=0; i<num; ++i) {
      active.segment[i] = start+i;
    }
    active.num = num;
  }

  template<typename Scene>
  inline void reset_deferred_buffers(const Scene& scene, by_material_t* deferred) {
    for (auto i=0; i<scene.materials.size(); ++i) {
      deferred[i].material   = scene.materials[i];
      deferred[i].splats.num = 0;
    }
  }

  template<typename Scene>
  inline void find_next_path_vertices(
    const Scene& scene
  , segment_t* segments
  , by_material_t* m
  , active_t& active
  , splat_t* splats)
  {
    // find intersection points following path vertices
    scene.intersect(segments, active);

    for (auto i=0; i<active.num; ++i) {
      auto  index   = active.segment[i];
      auto& segment = segments[index];
      if (segment.alive()) {
	auto mesh = scene.meshes[segment.mesh];

	segment.follow();
	segment.n = mesh->shading_normal(segment);

	auto& material = m[mesh->material->id];
	material.splats.segment[material.splats.num++] = index;
      }
    }
  }

  template<typename Scene>
  void snapshot(const Scene& scene) {
    const auto num_splats = film->num_splats();

    // automatically use all cores for now
    uint32_t cores = std::thread::hardware_concurrency();
    printf("Using %d threads for rendering\n", cores);

    std::thread threads[cores];

    for (auto t=0; t<cores; ++t) {
      threads[t] = std::thread([&]() {
	allocator_t allocator(1024*1024*100);
	Integrator  integrator(10);

	patch_t  patch;
	active_t active;

        while (film->next_patch(patch)) {
	  // allocate patch buffers
	  samples_t samples(allocator, num_splats);

	  auto segments = new(allocator) segment_t[num_splats];
	  auto deferred = new(allocator) by_material_t[scene.materials.size()];
	  auto splats   = new(allocator) splat_t[num_splats];

	  integrator.allocate(allocator, num_splats);

	  // sample all rays for this patch
	  sample_camera_vertices(patch, samples, segments, active, num_splats);

	  // TODO: find first hit separately and compute direct light contribution
	  // with stratified samples?

	  for (int i=0; i<num_splats; i+=SAMPLES_PER_ITERATION) {
	    activate_samples(active, i, SAMPLES_PER_ITERATION);

	    // run rendering pipeline for patch 
	    while (shading::has_live_paths(active)) {
	      reset_deferred_buffers(scene, deferred);
	      find_next_path_vertices(scene, segments, deferred, active, splats);

	      integrator.sample_lights(scene, segments, active);
	      active.clear();

	      auto m = deferred;
	      auto material_end = m+scene.materials.size();
	      do {
		if (shading::has_live_paths(m->splats)) {
		  auto bxdf = m->material->at(allocator);
		  integrator.shade(bxdf, segments, m->splats, splats);
		  integrator.sample_path_directions(bxdf, segments, m->splats, active);
		}
	      } while (++m != material_end);
	    }
	  }

	  film->apply_splats(patch, samples, splats);
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
