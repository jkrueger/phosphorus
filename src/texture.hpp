#pragma once

struct texture_t {
  enum class mode_t {
    CLAMP,
    REPEAT
  };

  mode_t umode, vmode;

  texture_t()
    : umode(REPEAT), vmode(REPEAT)
  {}
};
