#ifndef DCEARTH_RENDERER_HPP
#define DCEARTH_RENDERER_HPP

#include "arch/arch.h"
#include "dcearth/entity.hpp"
#include <dc/maple.h>
#include <dc/maple/controller.h>
#include <dc/maple/keyboard.h>
#include <dc/maple/mouse.h>
#include <dc/pvr.h>
#include <dc/video.h>

namespace dcearth {

class renderer {
public:
  static renderer &get() {
    static renderer instance;
    return instance;
  }

  void run() {
    pvr_init_defaults();
    if (scene_)
      scene_->on_enter();
    float dt = 1.0f / 60.0f;
    while (running_) {
      poll_input();
      if (scene_)
        scene_->on_update(dt);
      begin_frame();
      if (scene_)
        scene_->on_render(dt);
      end_frame();
    }
    if (scene_)
      scene_->on_exit();
    pvr_shutdown();
    arch_exit();
  }

  void quit() { running_ = false; }
  void set_scene(entity *s) { scene_ = s; }
  void set_bg_color(float r, float g, float b) { pvr_set_bg_color(r, g, b); }
  float screen_width() const { return (float)vid_mode->width; }
  float screen_height() const { return (float)vid_mode->height; }

private:
  renderer() = default;

  void begin_frame() {
    pvr_wait_ready();
    pvr_scene_begin();
  }

  void end_frame() {
    pvr_list_finish();
    pvr_scene_finish();
  }

  void poll_input() {
    if (!scene_)
      return;

    if (auto *cont = maple_enum_type(0, MAPLE_FUNC_CONTROLLER)) {
      if (auto *state = (cont_state_t *)maple_dev_status(cont)) {
        scene_->on_button(state->buttons);
        scene_->on_joystick(state->joyx / 128.0f, state->joyy / 128.0f,
                            state->joy2x / 128.0f, state->joy2y / 128.0f);
        scene_->on_trigger(state->ltrig / 255.0f, state->rtrig / 255.0f);
      }
    }

    if (auto *keyboard = maple_enum_type(0, MAPLE_FUNC_KEYBOARD)) {
      if (auto *state = kbd_get_state(keyboard)) {
        keyboard_input_state input;
        input.modifiers = state->cond.modifiers.raw;

        for (size_t i = 0; i < input.keys.size(); ++i)
          input.keys[i] = state->key_states[i].raw;

        scene_->on_keyboard(input);
      }
    }

    if (auto *mouse = maple_enum_type(0, MAPLE_FUNC_MOUSE)) {
      if (auto *state = (mouse_state_t *)maple_dev_status(mouse)) {
        mouse_input_state input;
        input.buttons = state->buttons;
        input.dx = state->dx;
        input.dy = state->dy;
        input.dz = state->dz;
        scene_->on_mouse(input);
      }
    }
  }

  entity *scene_ = nullptr;
  bool running_ = true;
};

} // namespace dcearth

#endif
