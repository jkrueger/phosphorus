#include "camera.hpp"
#include "integrator/path.hpp"
#include "thing.hpp"
#include "things/bvh.hpp"
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
  auto light0 = light_t::p(new light_t(shadable_t::p(new sphere_t({2,3,-3}, 0.25, white)), L));

  std::vector<thing_t::p> spheres;

  sample_t samples[256];
  sampling::strategies::stratified_2d(samples, 16);

  double dz = -4.0;
  for (auto j=0; j<16; ++j, dz += 1.0) {
    double dx = -4.0;
    for (auto i=0; i<16; ++i, dx += 1.0) {
      double x = dx + samples[j * 8 + i].u * 10.0;
      double z = dz + samples[j * 8 + i].v * 10.0;
      spheres.push_back(
        thing_t::p(
	  new sphere_t(
	    {x, -0.5 + rng::dis(rng::gen) * 2.0, z},
	    0.2, material_t::p(new diffuse_reflector_t({rng::dis(rng::gen), rng::dis(rng::gen), rng::dis(rng::gen)})))));
      dx += 0.5;
    }
    dz += 0.5;
  }

  auto plane0  = thing_t::p(new plane_t({0, -1.0, 0}, {0.0, 1.0, 0.0}, white));
  auto plane1  = thing_t::p(new plane_t({0,  0.0, 10.0}, {0.0, 0.0, -1.0}, white));
  auto plane2  = thing_t::p(new plane_t({-10.0,  0.0, 0}, {1.0, 0.0, 0.0}, white));
  auto plane3  = thing_t::p(new plane_t({10.0, 0.0, 0}, {-1.0, 0.0, 0.0}, white));
  things_t scene;
  bvh_t::p bvh(new bvh_t());
  printf("preprocessing\n");
  bvh->build(spheres);
  scene.add(plane0);
  scene.add(plane1);
  scene.add(plane2);
  scene.add(plane3);
  scene.add(light0);
  scene.add(bvh);

  camera_t<path_tracer_t> camera({0,0,0}, {0,0,1}, {0,1,0});
  camera.integrator.emitters.push_back(light0);
  printf("rendering\n");
  camera.snapshot(film, lens, scene);
  codec::image::exr::save("out.exr", film);
  printf("done\n");

  return 0;
}
