#include "camera.hpp"
#include "integrator/path.hpp"
#include "thing.hpp"
#include "things/bvh.hpp"
#include "things/sphere.hpp"
#include "things/plane.hpp"
#include "things/mesh.hpp"
#include "things/light.hpp"
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

const color_t L(16.0, 16.0, 16.0);

const material_t::p teal(new plastic_t({0.04, 0.47, 0.58}, {0.4,0.7,0.8}, 10.0));
const material_t::p red(new diffuse_reflector_t({1, 0.0, 0.0}));
const material_t::p pink(new diffuse_reflector_t({1, 0.4, 0.1}));
const material_t::p jade(new diffuse_reflector_t({0, 0.65, 0.41}));
const material_t::p orange(new diffuse_reflector_t({0.89, 0.52, 0.04}));
const material_t::p white(new diffuse_reflector_t({1, 1, 1}));
const material_t::p mirror(new mirror_t({0.5, 0.5, 0.5}));
const material_t::p glass(new glass_t({1, 0.23, 0.85}));

int main(int argc, char** argv) {
  stats_t::p stats(new stats_t());
  film_t film(WIDTH, HEIGHT, 16);
  lenses::pinhole_t lens;
  //auto light0 = light_t::p(new light_t(shadable_t::p(new sphere_t({800,500,-700}, 10.0, white)), L));
  auto light0 = light_t::p(new light_t(shadable_t::p(new sphere_t({0,2,-2}, 0.1, white)), L));
  printf("Loading mesh\n");

  std::vector<triangle_t::p> things;
  std::vector<mesh_t::p>     meshes;

  meshes.push_back(codec::mesh::ply::load("models/bunny.ply", white));
  meshes[0]->tesselate(things);
  auto plane0 = thing_t::p(new plane_t({0, -605.893, 0}, {0, -1.0, 0}, orange));
  auto plane1 = thing_t::p(new plane_t({0, 0, 605.893}, {0, 0, -1.0}, jade));
  auto plane2 = thing_t::p(new plane_t({0,  0.32, 0}, {0, 1, 0}, white));
  auto plane3 = thing_t::p(new plane_t({-66, 0, 0}, {1.0, 0.0, 0.0}, white));
  things_t scene;
  mesh_bvh_t::p bvh(new mesh_bvh_t());
  printf("preprocessing\n");
  bvh->build(things);
  //scene.add(plane0);
  //scene.add(plane1);
  scene.add(bvh);
  scene.add(plane2);
  //scene.add(plane3);
  scene.add(light0);

  auto camera = camera_t<path_tracer_t>::look_at(stats, {3, 1,-3}, {0,1,0});
  //auto camera = camera_t<path_tracer_t>::look_at(stats, {277,-300,250}, {-20,60,-20}, {1,0,0});
  //auto camera = camera_t<path_tracer_t>::look_at(stats, {450,1200,-500}, {400,0,-500}, {0,0,-1});
  camera->integrator.emitters.push_back(light0);

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
  std::cout << std::endl << "Hesperus is Venus" << std::endl;

  return 0;
}
