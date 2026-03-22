#ifndef DCEARTH_MAIN_SCENE_HPP
#define DCEARTH_MAIN_SCENE_HPP

#include "camera_entity.hpp"
#include "dcearth/light.hpp"
#include "dcearth/renderer.hpp"
#include "dcearth/scene.hpp"
#include "globe_entity.hpp"

namespace dcearth {

using namespace squint;

class main_scene : public scene {
public:
  directional_light sun;
  orbital_camera cam;
  globe_entity globe;

  void on_enter() override {
    renderer::get().set_bg_color(0.0f, 0.0f, 0.0f);
    sun.direction = normalize(vec3{0.5f, 0.8f, -0.3f});
    globe.cam = &cam;
    add_entity(&cam);
    add_entity(&globe);
  }

  void on_render(float dt) override {
    globe.mat.light_dir = sun.direction;
    pvr_list_begin(PVR_LIST_OP_POLY);
    scene::on_render(dt);
  }

  void on_button(uint32_t buttons) override {
    if ((buttons & CONT_RESET_BUTTONS) == CONT_RESET_BUTTONS)
      renderer::get().quit();
    scene::on_button(buttons);
  }
};

} // namespace dcearth

#endif
