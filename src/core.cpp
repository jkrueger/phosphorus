#include "camera.hpp"
#include "film.hpp"
#include "integrator/path.hpp"
#include "thing.hpp"
#include "things/mesh.hpp"
#include "things/tesselator.hpp"
#include "things/parametric.hpp"
#include "things/scene.hpp"
#include "traversal/bvh.hpp"
#include "material.hpp"
#include "material/diffuse.hpp"
#include "material/plastic.hpp"
#include "material/mirror.hpp"
#include "material/glass.hpp"
#include "material/paint.hpp"
#include "math/sampling.hpp"
#include "codec/image/exr.hpp"
#include "codec/mesh/ply.hpp"
#include "codec/scene.hpp"
#include "util/stats.hpp"

#include <dirent.h>
#include <sys/time.h>
#include <unistd.h>

typedef camera_t<film_t, lenses::pinhole_t, single_path_t> pinhole_camera_t;

const uint32_t film_t::PATCH_SIZE = 16;

const uint32_t WIDTH=1920;
const uint32_t HEIGHT=1080;

const color_t L(0.5, 0.5, 0.5);

const material_t::p dummy(new diffuse_reflector_t({0, 1, 0}, 40.0f));
const material_t::p discs(new diffuse_reflector_t({0.2, 0.2, 0.2}, 40.0f));
const material_t::p discs_texture(new diffuse_reflector_t({0.2, 0.2, 0.2}, 40.0f));
const material_t::p tubes(new diffuse_reflector_t({0.1, 0.1, 0.1}, 40.0f));
// const material_t::p paint(new plastic_t(color_t::from_rgb(9,7,7), color_t::from_rgb(4,3,3), .1f));
const material_t::p paint(new plastic_t(color_t::from_rgb(53, 35, 96), color_t::from_rgb(86, 54, 165), .9f));
const material_t::p application(new plastic_t({0.76f, 0.76f, 0.66f}, {0.66f, 0.66f, 0.66f}, 10.0));
const material_t::p backdrop(new diffuse_reflector_t({0.56f, 0.56f, 0.56f}, 80.0f));
const material_t::p logo(new diffuse_reflector_t({1.f, 0.0, 0.0}, 40.0));
const material_t::p breaks(new diffuse_reflector_t({1.f, 0.0, 0.0}, 40.0));
const material_t::p backlights(new diffuse_reflector_t({0.86f, 0.1f, 0.2f}, 40.0));
const material_t::p breaklights0(new diffuse_reflector_t({0.86f, 0.1f, 0.1f}, 40.0));
const material_t::p breaklights1(new diffuse_reflector_t({0.66f, 0.1f, 0.1f}, 40.0));
const material_t::p breaklights2(new diffuse_reflector_t({0.96f, 0.1f, 0.1f}, 40.0));
const material_t::p rubber(new diffuse_reflector_t(color_t::from_rgb(4, 4, 3), 80.0f));
const material_t::p tire0(new diffuse_reflector_t(color_t::from_rgb(4, 4, 3), 40.0f));
const material_t::p tire1(new diffuse_reflector_t(color_t::from_rgb(4, 4, 3), 40.0f));
const material_t::p black(new diffuse_reflector_t({0.05, 0.05, 0.05}, 40.0f));
const material_t::p interior(new diffuse_reflector_t(color_t::from_rgb(18, 18, 18), 40.0f));
const material_t::p under(new diffuse_reflector_t(color_t::from_rgb(37, 38, 38), 40.0f));
const material_t::p license(new plastic_t({0.66f, 0.66f, 0.66f}, {0.76f, 0.76f, 0.76f}, 10.0));
const material_t::p mirror(new mirror_t({1, 1, 1}));
const material_t::p chrome(new mirror_t({1, 1, 1}));
const material_t::p darkchrome(new mirror_t({0.5f, 0.5, 0.5f}));
const material_t::p glass(new glass_t({1, 1, 1}));
//const material_t::p backlights(new glass_t({0.86, 0.1, 0.2}));

int main(int argc, char** argv) {
  stats_t::p stats(new stats_t());

  auto path    = argv[1];
  auto samples = argc > 2 ? atoi(argv[2]) : 1;

  auto film    = film_t::p(new film_t(WIDTH, HEIGHT, samples));
  auto pinhole = lenses::pinhole_t::p(new lenses::pinhole_t);

  auto light0 = light_t::p(new light_t({-5.0f, 5.0f, -5.0f}, surface_t::p(new things::sphere_t(1.0)), L));

  mesh_scene_t scene(stats);
  scene.add(chrome);
  scene.add(black);
  scene.add(discs);
  scene.add(breaklights1);
  scene.add(paint);
  scene.add(interior);
  scene.add(glass);
  scene.add(under);
  scene.add(tire0);
  scene.add(rubber);
  scene.add(darkchrome);
  scene.add(tubes);
  scene.add(tire1);
  scene.add(dummy);
  scene.add(logo);
  scene.add(license);
  scene.add(breaks);
  scene.add(breaklights2);
  scene.add(discs_texture);
  scene.add(breaklights0);
  scene.add(backlights);
  scene.add(backdrop);
  scene.add(light0);

  codec::scene::load(path, scene);

  printf("Preprocessing geometry\n");
  scene.preprocess();

  pinhole_camera_t::p camera(new pinhole_camera_t(film, pinhole, stats));
  //camera->look_at({3, 3, -3}, {0,0.7,0});
  camera->look_at({-2.5, 0.4f, 3}, {0,0,1});
  //auto camera = camera_t<path_tracer_t>::look_at(stats, {277,-300,250}, {-20,60,-20}, {1,0,0});
  //auto camera = camera_t<path_tracer_t>::look_at(stats, {450,1200,-500}, {400,0,-500}, {0,0,-1});

  auto done = false;
  auto t = std::thread([&](){
    timeval start;
    gettimeofday(&start, 0);
    while (!done) {
      usleep(1000000);
      timeval now;
      gettimeofday(&now, 0);
      auto progress = (((float)stats->areas / (float)film->num_patches) * 100.0f);
      std::cout
	<< "\rprogess: " << progress
	<< ", rays/s: " << stats->rays / (now.tv_sec - start.tv_sec)
	<< std::flush;
    }
  });

  printf("Rendering\n");
  
  // rendering starts here
  timeval start, end;
  gettimeofday(&start, 0);

  camera->snapshot(scene);
  done = true;

  gettimeofday(&end, 0);
  
  if (t.joinable()) {
    t.join();
  }

  float time = (end.tv_sec - start.tv_sec) + ((end.tv_usec - start.tv_usec) / 1000000.0);

  codec::image::exr::save("out.exr", film);

  std::cout << std::endl << "rendering time: " << time << std::endl;
  std::cout << std::endl << "Phosphoros is Venus" << std::endl;

  return 0;
}
