#pragma once

#include <string>

struct film_t;

namespace codec {
  namespace image {
    namespace exr {
      void save(const std::string& path, const std::shared_ptr<film_t>& film);
    };
  }
}
