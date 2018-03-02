#pragma once

#include <string>

struct scene_t;

namespace codec {
  namespace scene {
    void load(const std::string& path, scene_t& scene);
  }
}
