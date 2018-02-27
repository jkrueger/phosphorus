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

const uint32_t WIDTH=1280;
const uint32_t HEIGHT=720;

const color_t L(0.5, 0.5, 0.5);

const material_t::p dummy0(new diffuse_reflector_t({1, 0, 0}, 40.0f));
const material_t::p dummy1(new diffuse_reflector_t({0, 1, 0}, 40.0f));
const material_t::p dummy2(new diffuse_reflector_t({1, 1, 0}, 40.0f));
const material_t::p dummy3(new diffuse_reflector_t({0, 1, 1}, 40.0f));
const material_t::p dummy4(new diffuse_reflector_t({1, 0, 1}, 40.0f));
const material_t::p dummy5(new diffuse_reflector_t({1, 1, 1}, 40.0f));
const material_t::p dummy6(new diffuse_reflector_t({0.5f, 0.5f, 0.5f}, 40.0f));
const material_t::p dummy7(new diffuse_reflector_t({0.8f, 0.2f, 0.6f}, 40.0f));
const material_t::p dummy8(new diffuse_reflector_t({0.8f, 0.6f, 0.2f}, 40.0f));
const material_t::p paint(new paint_t({0.86f, 0.86f, 0.86f}));
const material_t::p rims(new plastic_t({0.76f, 0.76f, 0.66f}, {0.66f, 0.66f, 0.66f}, 10.0));
const material_t::p backdrop(new diffuse_reflector_t({0.56f, 0.56f, 0.56f}, 80.0f));
const material_t::p logo(new diffuse_reflector_t({1.f, 0.0, 0.0}, 40.0));
const material_t::p breaks(new diffuse_reflector_t({1.f, 0.0, 0.0}, 40.0));
const material_t::p backlights(new diffuse_reflector_t({0.86f, 0.1f, 0.2f}, 40.0));
const material_t::p breaklights(new diffuse_reflector_t({0.86f, 0.1f, 0.1f}, 40.0));
const material_t::p rubber(new diffuse_reflector_t(color_t::from_rgb(17, 18, 18), 40.0f));
const material_t::p tire0(new diffuse_reflector_t(color_t::from_rgb(17, 18, 18), 40.0f));
const material_t::p tire1(new diffuse_reflector_t(color_t::from_rgb(17, 18, 18), 40.0f));
const material_t::p black(new diffuse_reflector_t(color_t::from_rgb(37, 38, 38), 40.0f));
const material_t::p interior(new diffuse_reflector_t(color_t::from_rgb(128, 128, 128), 40.0f));
const material_t::p under(new diffuse_reflector_t(color_t::from_rgb(37, 38, 38), 40.0f));
const material_t::p license(new plastic_t({0.76f, 0.76f, 0.66f}, {0.66f, 0.66f, 0.66f}, 10.0));
//const material_t::p teal2(new plastic_t({0.04, 0.47, 0.58}, {0.4,0.7,0.8}, 100.0));
//const material_t::p pink(new diffuse_reflector_t({1.f, 0.4, 0.1}));
const material_t::p purple(new plastic_t(R*0.2f, R*0.4f, .1f));
//const material_t::p jade(new diffuse_reflector_t({0.f, 0.65, 0.41}));
//const material_t::p orange(new diffuse_reflector_t({0.89, 0.52, 0.04}, 2.0f));
const material_t::p mirror(new mirror_t({1, 1, 1}));
const material_t::p darkchrome(new mirror_t({0.5f, 0.5, 0.5f}));
const material_t::p glass(new glass_t({0.86, 0.86, 0.86}));
//const material_t::p backlights(new glass_t({0.86, 0.1, 0.2}));

int main(int argc, char** argv) {
  stats_t::p stats(new stats_t());

  auto path    = argv[1];
  auto samples = argc > 2 ? atoi(argv[2]) : 1;

  auto film    = film_t::p(new film_t(WIDTH, HEIGHT, samples));
  auto pinhole = lenses::pinhole_t::p(new lenses::pinhole_t);

  auto light0 = light_t::p(new light_t({-5.0f, 5.0f, -5.0f}, surface_t::p(new things::sphere_t(1.0)), L));

  mesh_scene_t scene(stats);
  scene.add(backdrop);
  scene.add(paint);
  scene.add(dummy0);
  scene.add(rubber);
  scene.add(glass);
  scene.add(rims);
  scene.add(backlights);
  scene.add(interior);
  scene.add(logo);
  scene.add(breaks);
  scene.add(tire0);
  scene.add(dummy3);
  scene.add(under);
  scene.add(tire1);
  scene.add(license);
  scene.add(darkchrome);
  scene.add(black);
  scene.add(dummy5);
  scene.add(dummy6);
  scene.add(breaklights);
  scene.add(dummy8);
  scene.add(light0);

  codec::scene::load(path, scene);

  printf("Preprocessing geometry\n");
  scene.preprocess();

  pinhole_camera_t::p camera(new pinhole_camera_t(film, pinhole, stats));
  //camera->look_at({3, 3, -3}, {0,0.7,0});
  camera->look_at({-1, 0.5, -4}, {0,-0.1,1});
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
