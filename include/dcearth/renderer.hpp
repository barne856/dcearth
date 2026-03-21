#ifndef DCEARTH_RENDERER_HPP
#define DCEARTH_RENDERER_HPP

#include "dcearth/entity.hpp"
#include <dc/maple.h>
#include <dc/maple/controller.h>
#include <dc/pvr.h>

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
    }

    void quit() { running_ = false; }
    void set_scene(entity_base *s) { scene_ = s; }
    void set_bg_color(float r, float g, float b) { pvr_set_bg_color(r, g, b); }

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
        auto *cont = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);
        if (!cont || !scene_)
            return;
        auto *state = (cont_state_t *)maple_dev_status(cont);
        if (!state)
            return;
        scene_->on_button(state->buttons);
        scene_->on_joystick(state->joyx / 128.0f, state->joyy / 128.0f, state->joy2x / 128.0f, state->joy2y / 128.0f);
        scene_->on_trigger(state->ltrig / 255.0f, state->rtrig / 255.0f);
    }

    entity_base *scene_ = nullptr;
    bool running_ = true;
};

} // namespace dcearth

#endif
