#include "material.hpp"

uint32_t material_t::ids = 0;

material_t::material_t()
  : id(ids++) {
}
