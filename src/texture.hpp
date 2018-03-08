#pragma once

#include "shading.hpp"
#include "util/color.hpp"

#include <memory>
#include <string>

template<typename T>
struct texture_t
{
  typedef texture_t* p;
  
  enum wrap_mode_t {
    Default,
    Black,
    Clamp,
    Periodic,
    Mirror
  };

  struct impl_t;
  std::unique_ptr<impl_t> impl;
  
  /**
   * Evaluate the texture for a surface point
   *
   */
  T eval(const segment_t& s) const;

  /**
   * Evalualte the texture for a vector d, effectively
   * doing an environment map lookup
   *
   */
  T eval(const vector_t& d) const;

  // loader functions

  static p constant(const T& c);
  static p load(const std::string& path);

  static void boot();
  static void attach();

private:
  // private constructor from impl pointer
  texture_t(impl_t* impl);
};
