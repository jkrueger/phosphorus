#include "camera.hpp"
#include "integrator/path.hpp"
#include "thing.hpp"
#include "things/sphere.hpp"
#include "things/plane.hpp"
#include "things/light.hpp"
#include "material.hpp"
#include "material/diffuse.hpp"
#include "material/plastic.hpp"
#include "material/mirror.hpp"
#include "material/glass.hpp"
#include "math/sampling.hpp"
#include "codec/image/exr.hpp"

const uint32_t WIDTH=1024;
const uint32_t HEIGHT=768;

const color_t L(8.0, 8.0, 8.0);

const material_t::p teal(new plastic_t({0.04, 0.47, 0.58}, {0.4,0.7,0.8}, 40.0));
const material_t::p pink(new diffuse_reflector_t({1, 0.23, 0.85}));
const material_t::p orange(new diffuse_reflector_t({0.89, 0.52, 0.04}));
const material_t::p white(new diffuse_reflector_t({1, 1, 1}));
const material_t::p mirror(new mirror_t({0.5, 0.5, 0.5}));
const material_t::p glass(new glass_t({1, 0.23, 0.85}));

int main(int argc, char** argv) {
  film_t film(WIDTH, HEIGHT, 16);
  lenses::pinhole_t lens;
  auto light0  = light_t::p(new light_t(shadable_t::p(new sphere_t({2,3,-3}, 0.25, white)), L));
  auto light1  = light_t::p(new light_t(shadable_t::p(new sphere_t({-4,2,-3}, 0.25, white)), L));
  auto sphere0 = thing_t::p(new sphere_t({0, 0, 0}, 1.0, orange));
  auto sphere1 = thing_t::p(new sphere_t({-2.5, 1, 2}, 2.0, teal));
  auto sphere2 = thing_t::p(new sphere_t({-1, -0.5, -2}, 0.5, glass));
  auto plane0  = thing_t::p(new plane_t({0, -1.0, 0}, {0.0, 1.0, 0.0}, white));
  auto plane1  = thing_t::p(new plane_t({0, 0.0, 10.0}, {0.0, 0.0, -1.0}, white));
  things_t scene;
  scene.things.push_back(plane0);
  scene.things.push_back(plane1);
  scene.things.push_back(sphere0);
  scene.things.push_back(sphere1);
  scene.things.push_back(sphere2);
  scene.things.push_back(light0);
  // scene.things.push_back(light1);

  camera_t<path_tracer_t> camera({0,0,0}, {0,0,1}, {0,1,0});
  camera.integrator.emitters.push_back(light0);
  //camera.integrator.emitters.push_back(light1);
  //camera.integrator.emitters.push_back(sphere2);

  camera.snapshot(film, lens, scene);
  codec::image::exr::save("out.exr", film);
  printf("done\n");

  return 0;
}
