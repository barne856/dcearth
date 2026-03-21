#ifndef DCEARTH_CAMERA_ENTITY_HPP
#define DCEARTH_CAMERA_ENTITY_HPP

#include "dcearth/entity.hpp"
#include <squint/squint.hpp>

namespace dcearth {

using namespace squint;
using namespace squint::geometry;

class camera_entity : public entity<camera_entity> {
public:
  mat4 view = mat4::eye();
  mat4 projection = mat4::eye();

  void on_enter() override {
    projection = perspective(float(M_PI) / 4.0f, 640.0f / 480.0f, length{0.1f},
                             length{100.0f});
    view = mat4::eye();
    translate(view, vec3_t<length>{length{0.0f}, length{0.0f}, length{-4.0f}});
  }
};

} // namespace dcearth

#endif
