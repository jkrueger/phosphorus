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
#include "math/sampling.hpp"
#include "codec/mesh/ply.hpp"

#include <dirent.h>
#include <sys/time.h>
#include <unistd.h>

const uint32_t WIDTH=1280;
const uint32_t HEIGHT=720;

const color_t L(32.0, 32.0, 32.0);

const color_t white(0.96f, 0.96f, 0.86f);
const color_t teal(0.04, 0.47, 0.58);

int main(int argc, char** argv) {
  stats_t::p stats(new stats_t());

  film_t film(WIDTH, HEIGHT, 1);
  lenses::pinhole_t lens;

  scene_t<mesh_bvh_t> scene(stats);

  const auto matte_white = scene.material<diffuse_reflector_t>(white, 40.0f);
  const auto matte_teal  = scene.material<diffuse_reflector_t>(teal, 40.0f);

  const auto surface = surface_t::p(new things::rectangle_t(0.4)); 
  const auto light   = scene.light({0.0f, 4.0f, 0.0f}, surface, L));

  mesh_t::p floor(tesselate::surface(parametric::rectangle_t{100, 100}, matte_teal));
  mesh_t::p bunny(codec::mesh::ply::load("models/bunny.ply", matte_white));

  scene.add(light0);
  scene.add(floor);
  scene.add(bunny);

  scene.preprocess();

  auto camera = camera_t<single_path_t>::look_at(stats, {0, 3, -5}, {0,0.7,0});

  bool done = false;
  while (!done) {
    camera->snapshot(film, lens, scene);
    // display
  }
  return 0;
}
