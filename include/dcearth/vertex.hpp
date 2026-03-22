#ifndef DCEARTH_VERTEX_HPP
#define DCEARTH_VERTEX_HPP

#include <squint/squint.hpp>

namespace dcearth {

using namespace squint;

struct vertex {
  vec3 position;
  vec3 normal;
  vec2 uv;
};

} // namespace dcearth

#endif
