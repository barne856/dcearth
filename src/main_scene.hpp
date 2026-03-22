#ifndef DCEARTH_MAIN_SCENE_HPP
#define DCEARTH_MAIN_SCENE_HPP

#include "camera_entity.hpp"
#include "dcearth/light.hpp"
#include "dcearth/renderer.hpp"
#include "dcearth/scene.hpp"
#include "globe_entity.hpp"

namespace dcearth {

using namespace squint;

class main_scene : public scene<main_scene> {
public:
  directional_light sun;
  orbital_camera cam;
  globe_entity globe;

  void on_enter() override {
    renderer::get().set_bg_color(0.0f, 0.0f, 0.0f);
    sun.direction = normalize(vec3{0.5f, 0.8f, -0.3f});
    cam.attach_controls_system<orbital_controls>();
    globe.cam = &cam;
    add_entity(&cam);
    add_entity(&globe);
  }

  void render(float) {
    globe.mat.light_dir = sun.direction;
    pvr_list_begin(PVR_LIST_OP_POLY);
  }

  void button(uint32_t buttons) {
    if ((buttons & CONT_RESET_BUTTONS) == CONT_RESET_BUTTONS)
      renderer::get().quit();
  }
};

} // namespace dcearth

#endif
