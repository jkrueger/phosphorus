#pragma once

#include "texture.hpp"

#include <tuple>

namespace textures {
  struct image_t : public texture_t {
    float*   data;
    uint32_t width, height;

    image_texture_t()
      : width(0), height(0), bytes(0)
    {}

    image_texture_t(uint32_t width, uint32_t height)
      width(width), height(height)
    {}

    inline color_t sample(float u, float v/* , du, dv */) const {
    
    }

    template<typename Codec>
    static inline image_texture_t::p from(const std::string& path, const Codec& codec) const {
      image_texture_t::p tex(new image_texture_t());
      std::tie(tex->width, tex->height, tex->bytes) = codec(path);
      return tex;
    }
  };
}
