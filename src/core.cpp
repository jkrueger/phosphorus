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

const uint32_t WIDTH=1024;
const uint32_t HEIGHT=768;

const color_t L(32.0f, 32.0f, 32.0f);

const material_t::p def(new diffuse_reflector_t({1, 1, 1}, 0));
//const material_t::p bottom(new plastic_t({0.1, 0.1, 0.1}, {.7,.7,.7}, .1f));
const material_t::p bottom(new diffuse_reflector_t({1, 1, 1}, 0));
const material_t::p left(new diffuse_reflector_t({0.8, 0.072, 0.111}, 0));
const material_t::p right(new diffuse_reflector_t({0.142, 0.148, 0.8}, 0));
const material_t::p mirror(new mirror_t({1, 1, 1}));
const material_t::p glass(new glass_t({1, 1, 1}));
const material_t::p test(new paint_t({0.72,0.1,0.65}));
const material_t::p test2(new plastic_t({0.2,0.2,0.2}, {1,1,1}, 0.1f));

int main(int argc, char** argv) {
  stats_t::p stats(new stats_t());

  auto path    = argv[1];
  auto samples = argc > 2 ? atoi(argv[2]) : 1;

  auto film    = film_t::p(new film_t(WIDTH, HEIGHT, samples));
  auto pinhole = lenses::pinhole_t::p(new lenses::pinhole_t);
  auto light0  = light_t::p(new light_t({0, 2.3f, 0}, surface_t::p(new things::sphere_t(0.05f)), L));

  mesh_scene_t scene(stats);
  scene.add(def);
  scene.add(left);
  scene.add(right);
  scene.add(test);
  scene.add(bottom);
  scene.add(mirror);
  scene.add(test2);
  scene.add(light0);

  codec::scene::load(path, scene);

  printf("Preprocessing geometry\n");
  scene.preprocess();

  pinhole_camera_t::p camera(new pinhole_camera_t(film, pinhole, stats));
  camera->look_at({0, 1.25f, -3.8}, {0,1.25f,0});

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
