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

  typedef typename Film::splat_t splat_t;
  typedef typename Film::patch_t patch_t;

  vector_t          position;
  orthogonal_base_t orientation;

  Integrator integrator;

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
    , integrator(8)
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

  template<typename Patch>
  inline void sample_camera_vertices(
    const Patch& patch
  , segment_t* segments
  , active_t& active
  , uint32_t num_splats) const
  {
    const auto ratio = (float_t) film->width / (float_t) film->height;
    const auto stepx = 1.0f/film->width;
    const auto stepy = 1.0f/film->height;

    auto splat   = 0;
    auto segment = segments;
    for (auto y=patch.y; y<patch.yend(); ++y) {
      for (auto x=patch.x; x<patch.xend(); ++x) {
	auto ndcx = (-0.5f + x * stepx) * ratio;
	auto ndcy = 0.5f - y * stepy;

	for (auto i=0; i<film->samples; ++i, ++segment, ++splat) {
	  auto sx = samples[i].u - 0.5f;
	  auto sy = samples[i].v - 0.5f;

	  segment->p  = position;
	  segment->wi =
	    orientation.to_world({
	      sx * stepx + ndcx
	    , sy * stepy + ndcy
	    , 1.0f
	    }).normalize();

	  active.segment[splat] = splat;
	};
      }
    }

    active.num = num_splats;
  }

  template<typename Scene>
  inline void reset_deferred_buffers(const Scene& scene, by_material_t* deferred) {
    for (auto i=0; i<material_t::ids; ++i) {
      deferred[i].material = scene.materials[i];
      deferred[i].splats.num = 0;
    }
  }

  template<typename Scene>
  inline void find_next_path_vertices(
    const Scene& scene
  , segment_t* segments
  , by_material_t* m
  , active_t& active)
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
    uint32_t cores = 1; //std::thread::hardware_concurrency();
    printf("Using %d threads for rendering\n", cores);

    std::thread threads[cores];

    for (auto t=0; t<cores; ++t) {
      threads[t] = std::thread([&]() {
	allocator_t allocator(1024*1024*100);

	samples_t samples; // the current set of film samples we're rendering
	active_t  active;  // active path vertices

        while (film->next_patch(patch)) {
	  // allocate patch buffers
	  segment_t*     segments = new(allocator) segment_t[num_splats];
	  by_material_t* deferred = new(allocator) by_material_t[material_t::ids];
	  splat_t*       splats   = new(allocator) splat_t[num_splats];

	  // sample all rays for this patch
	  sample_camera_vertices(patch, segments, active, num_splats);

	  // TODO: find first hit separately and compute direct light contribution
	  // with stratified samples?

	  // run rendering pipeline for patch 
	  while (shading::has_live_paths(active)) {
	    reset_deferred_buffers(scene, deferred);
	    find_next_path_vertices(scene, segments, deferred, active);

	    integrator.sample_lights(scene, segments, active);
	    active.clear();

	    auto m = deferred;
	    auto material_end = m+material_t::ids;
	    do {
	      if (shading::has_live_paths(m->splats)) {
		auto bxdf = m->material->at(allocator);
		integrator.shade(bxdf, segments, m->splats, splats);
		integrator.sample_path_directions(bxdf, segments, m->splats, active);
	      }
	    } while (++m != material_end);
	  }

	  film->apply_splats(patch, splats);
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
