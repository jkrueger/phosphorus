#include "camera.hpp"
#include "integrator/path.hpp"
#include "thing.hpp"
#include "things/sphere.hpp"
#include "things/plane.hpp"
#include "things/light.hpp"
#include "material.hpp"
#include "material/diffuse.hpp"
#include "math/sampling.hpp"
#include "codec/image/exr.hpp"

const uint32_t WIDTH=1024;
const uint32_t HEIGHT=768;

const color_t L(16.0, 16.0, 16.0);

const material_t::p teal(new diffuse_reflector_t({0.04, 0.47, 0.58}));
const material_t::p pink(new diffuse_reflector_t({1, 0.23, 0.85}));
const material_t::p white(new diffuse_reflector_t({1, 1, 1}));

int main(int argc, char** argv) {
  film_t film(WIDTH, HEIGHT);
  lenses::pinhole_t lens;
  auto light0  = light_t::p(new light_t(thing_t::p(new sphere_t({1,1,-3}, 0.25, white)), L));
  auto light1  = light_t::p(new light_t(thing_t::p(new sphere_t({-3,1,-5}, 0.25, white)), L));
  auto sphere0 = thing_t::p(new sphere_t({0, 0, 0}, 1.0, white));
  auto sphere1 = thing_t::p(new sphere_t({-2.5, 1, 2}, 2.0, teal));
  auto sphere2 = thing_t::p(new sphere_t({-1, -0.75, -1}, 0.25, pink));
  auto plane0  = thing_t::p(new plane_t({0, -1.0, 0}, {0.0, 1.0, 0.0}, white));
  auto plane1  = thing_t::p(new plane_t({0, 0.0, 3.0}, {0.0, 0.0, -1.0}, white));
  things_t scene;
  scene.things.push_back(plane0);
  //scene.things.push_back(plane1);
  scene.things.push_back(sphere0);
  scene.things.push_back(sphere1);
  scene.things.push_back(sphere2);
  scene.things.push_back(light0);
  scene.things.push_back(light1);

  camera_t<path_tracer_t> camera({0,0,0}, {0,0,1}, {0,1,0});
  camera.integrator.emitters.push_back(light0);
  camera.integrator.emitters.push_back(light1);

  camera.snapshot(film, lens, scene);
  codec::image::exr::save("out.exr", film);

  return 0;
}
