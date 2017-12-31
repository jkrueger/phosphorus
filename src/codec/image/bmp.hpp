#pragma once

#include <string>

struct film_t;

namespace codec {
  namespace image {
    namespace bmp {
      void save(const std::string& path, const film_t& film);
    };
  }
}
