#ifndef DCEARTH_MAIN_SCENE_HPP
#define DCEARTH_MAIN_SCENE_HPP

#include "camera_entity.hpp"
#include "dcearth/light.hpp"
#include "dcearth/renderer.hpp"
#include "dcearth/scene.hpp"
#include "earth_entity.hpp"

namespace dcearth {

using namespace squint;

class main_scene;

class scene_render_system : public render_system<main_scene> {
public:
  void render(float dt, main_scene &s) override;
};

class main_scene : public scene<main_scene> {
public:
  directional_light sun;
  camera_entity cam;
  earth_entity earth;

  void on_enter() override;

  void button(uint32_t buttons) {
    if ((buttons & CONT_RESET_BUTTONS) == CONT_RESET_BUTTONS)
      renderer::get().quit();
  }
};

inline void scene_render_system::render(float dt, main_scene &s) {
  mat4 vp = s.cam.projection * s.cam.view;
  s.earth.mat.light_dir = s.sun.direction;
  s.earth.mat.light_color = s.sun.color;
  s.earth.sphere.draw(&s.earth.mat, vp * s.earth.model);
}

inline void main_scene::on_enter() {
  renderer::get().set_bg_color(0.05f, 0.05f, 0.1f);
  sun.direction = normalize(vec3{0.5f, 0.8f, -0.3f});
  sun.color = vec3{1.0f, 0.6f, 0.05f};
  attach_render_system<scene_render_system>();
  add_entity(&cam);
  add_entity(&earth);
}

} // namespace dcearth

#endif
