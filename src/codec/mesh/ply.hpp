#pragma once

#include "things/mesh.hpp"

#include <string>

namespace codec {
  namespace mesh {
    namespace ply {
      mesh_t::p load(const std::string& path, const material_t::p&);
    }
  }
}
