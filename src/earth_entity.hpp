#ifndef DCEARTH_EARTH_ENTITY_HPP
#define DCEARTH_EARTH_ENTITY_HPP

#include "dcearth/entity.hpp"
#include "dcearth/mesh.hpp"
#include <squint/squint.hpp>

namespace dcearth {

using namespace squint;
using namespace squint::geometry;

class earth_entity : public entity<earth_entity> {
  public:
    sphere_mesh sphere{1.0f, 24, 16};
    color_material mat;
    mat4 model = mat4::eye();
    float angle = 0.0f;

    void on_enter() override { mat.base_color = {0x30 / 255.0f, 0x80 / 255.0f, 0xff / 255.0f}; }

    void update(float dt) {
        angle += dt;
        model = mat4::eye();
        rotate(model, angle, vec3{0.0f, 1.0f, 0.0f});
        rotate(model, 0.3f, vec3{1.0f, 0.0f, 0.0f});
    }
};

} // namespace dcearth

#endif
