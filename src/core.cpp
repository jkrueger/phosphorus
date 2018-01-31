#include "camera.hpp"
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
#include "math/sampling.hpp"
#include "codec/image/exr.hpp"
#include "codec/mesh/ply.hpp"
#include "util/stats.hpp"

#include <dirent.h>
#include <sys/time.h>
#include <unistd.h>

const uint32_t WIDTH=1280;
const uint32_t HEIGHT=720;

const color_t L(64.0, 64.0, 64.0);

const material_t::p white(new diffuse_reflector_t({1.0f, 1.0f, 1.0f}));
const material_t::p teal(new diffuse_reflector_t({0.04, 0.47, 0.58}));
const material_t::p teal2(new plastic_t({0.04, 0.47, 0.58}, {0.4,0.7,0.8}, 40.0));
const material_t::p red(new diffuse_reflector_t({1.f, 0.0, 0.0}));
const material_t::p pink(new diffuse_reflector_t({1.f, 0.4, 0.1}));
const material_t::p purple(new diffuse_reflector_t(color_t::from_rgb(70, 33, 122)));
const material_t::p jade(new diffuse_reflector_t({0.f, 0.65, 0.41}));
const material_t::p orange(new diffuse_reflector_t({0.89, 0.52, 0.04}));
const material_t::p mirror(new mirror_t({1, 1, 1}));
const material_t::p glass(new glass_t({1, 1, 1}));

int main(int argc, char** argv) {
  stats_t::p stats(new stats_t());
  film_t film(WIDTH, HEIGHT, 4);
  lenses::pinhole_t lens;
  auto light0 = light_t::p(new light_t({0.0f, 4.0f, 0.0f}, surface_t::p(new things::sphere_t(0.2)), L));
  printf("Loading mesh\n");
  mesh_t::p floor(tesselate::surface(parametric::rectangle_t{100, 100}, orange));
  mesh_t::p bunny(codec::mesh::ply::load("models/bunny.ply", white));

  printf("preprocessing\n");
  scene_t<mesh_bvh_t> scene(stats);
  scene.add(light0);
  scene.add(bunny);
  scene.add(floor);
  scene.preprocess();

  auto camera = camera_t<single_path_t>::look_at(stats, {3, 3, -3}, {0,0.7,0});
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
	auto progress = (((float)stats->areas / (float)film.num_areas) * 100.0f);
	std::cout
	  << "\rprogess: " << progress
	  << ", rays/s: " << stats->rays / (now.tv_sec - start.tv_sec)
	  << std::flush;
      }
    });

  // rendering starts here
  timeval start, end;
  gettimeofday(&start, 0);

  camera->snapshot(film, lens, scene);
  codec::image::exr::save("out.exr", film);
  done = true;

  gettimeofday(&end, 0);
  
  if (t.joinable()) {
    t.join();
  }

  float time = (end.tv_sec - start.tv_sec) + ((end.tv_usec - start.tv_usec) / 1000000.0);

  std::cout << std::endl << "rendering time: " << time << std::endl;
  std::cout << std::endl << "Phosphoros is Venus" << std::endl;

  return 0;
}
