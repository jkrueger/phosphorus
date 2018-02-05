#include "exr.hpp"
#include "camera.hpp"
#include "film.hpp"

#pragma clang diagnostic ignored "-Wdeprecated-register"
#include "OpenEXR/ImfRgbaFile.h"

using namespace Imf;

void codec::image::exr::save(const std::string& path, const film_t& film) {
  Rgba data[film.width*film.height];
  for (auto y=0; y<film.height; ++y) {
    for (auto x=0; x<film.width; ++x) {
      auto& pixel = film.pixel(x, y);
      data[y*film.width+x] = {(float)pixel.r, (float)pixel.g, (float)pixel.b, 1.0f};
    }
  }
  RgbaOutputFile file(path.c_str(), film.width, film.height, WRITE_RGBA);
  file.setFrameBuffer(data, 1, film.width);
  file.writePixels(film.height);
}
