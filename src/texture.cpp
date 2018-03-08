#include "texture.hpp"

#pragma clang diagnostic ignored "-Wdeprecated-register"
#include <OpenImageIO/texture.h>
#include <OpenEXR/ImathVec.h>

using namespace OpenImageIO_v1_9;

template<>
struct texture_t<color_t>::impl_t {
  virtual color_t eval(const segment_t&) const = 0;
  virtual color_t eval(const vector_t&) const = 0;
};

struct oiio_t : public texture_t<color_t>::impl_t {
  static TextureSystem* oiio;
  static thread_local TextureSystem::Perthread* thread_info; 

  TextureSystem::TextureHandle* handle;

  oiio_t(const std::string& path) {
    handle = oiio->get_texture_handle(ustring(path.c_str()));
    if (!oiio->good(handle)) {
      throw std::runtime_error("Can't load texture: " + path);
    }
  }

  color_t eval(const segment_t& segment) const {
    color_t out;

    TextureOpt opts;

    oiio->texture(handle, thread_info, opts, segment.u, segment.v, 0, 0, 0, 0, 3, out.v);
    return out;
  }

  color_t eval(const vector_t& d) const {
    color_t out;

    TextureOpt opts;

    oiio->environment(
      handle
    , thread_info
    , opts
    , Imath::V3f(d.x, d.y, d.z)
    , Imath::V3f(0), Imath::V3f(0)
    , 3
    , out.v);

    return out;
  }

  static void boot() {
    oiio = TextureSystem::create();
  }

  static void attach() {
    thread_info = oiio->get_perthread_info();
  }
};

template<typename T>
struct constant_t : public texture_t<T>::impl_t
{
  T c;

  constant_t(const T& t)
    : c(t)
  {}

  T eval(const segment_t&) const {
    return c;
  }
  
  T eval(const vector_t&) const {
    return c;
  }
};


template<>
texture_t<color_t>::texture_t(impl_t* impl)
  : impl(impl)
{}

template<>
color_t texture_t<color_t>::eval(const segment_t& segment) const {
  return impl->eval(segment);
}

template<>
color_t texture_t<color_t>::eval(const vector_t& d) const {
  return impl->eval(d);
}

template<>
texture_t<color_t>::p texture_t<color_t>::constant(const color_t& c) {
  return texture_t::p(new texture_t(new constant_t<color_t>(c)));
}

template<>
texture_t<color_t>::p texture_t<color_t>::load(const std::string& path) {
  return texture_t::p(new texture_t(new oiio_t(path)));
}

template class texture_t<color_t>;
