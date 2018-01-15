#include "bmp.hpp"
#include "camera.hpp"
#include "util/algo.hpp"

#include <algorithm>
#include <fstream>
#include <stdexcept>

#define BMP_MAGIC 0x4D42

struct header_t {
  uint16_t magic;
  uint32_t file_size;
  uint32_t reserved;
  uint32_t data_offset;
} __attribute__((__packed__ ));

struct info_header_t {
  uint32_t info_size;
  int32_t width;
  int32_t height;
  uint16_t plane_count;
  uint16_t bpp;
  uint32_t compression;
  uint32_t image_size;
  int32_t h_dpm;
  int32_t v_dpm;
  uint32_t colour_count;
  uint32_t imp_colour_count;
} __attribute__((__packed__ ));

void codec::image::bmp::save(const std::string& path, const film_t& film) {
  std::fstream file(path, std::fstream::out | std::fstream::binary);
  if (!file.good()) {
    throw std::runtime_error("Failed to open file for output: " + path);
  }

  auto line_size = 4 * film.width;
  auto size = line_size * film.height;
  
  header_t header = {
    BMP_MAGIC,
    sizeof(header_t),
    0,
    sizeof(header_t) + sizeof(info_header_t)
  };

  info_header_t info = {
    sizeof(info_header_t),
    (int32_t) film.width, (int32_t) film.height,
    1, 32, 0, size,
    0, 0, 0, 0
  };
  
  file.write((const char*) &header, sizeof(header_t));
  file.write((const char*) &info, sizeof(info_header_t));

  unsigned char data[size];
  for (auto y=0; y<film.height; ++y) {
    for (auto x=0; x<film.width; ++x) {
      auto index = y*line_size+(x*4);
      auto color = film.pixel(x, film.height - (y + 1));
      data[index    ] = clamp(color.b, 0.0f, 1.0f) * 255.0;
      data[index + 1] = clamp(color.g, 0.0f, 1.0f) * 255.0;
      data[index + 2] = clamp(color.r, 0.0f, 1.0f) * 255.0;
      data[index + 3] = 255;
    }
  }

  file.write((const char*) data, size);
}
