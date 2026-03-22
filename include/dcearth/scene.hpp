#ifndef DCEARTH_SCENE_HPP
#define DCEARTH_SCENE_HPP

#include "dcearth/entity.hpp"
#include <vector>

namespace dcearth {

class scene : public entity {
public:
  void on_exit() override {
    for (auto *e : entities_)
      e->on_exit();
  }

  void on_render(float dt) override {
    for (auto *e : entities_)
      e->on_render(dt);
  }

  void on_update(float dt) override {
    for (auto *e : entities_)
      e->on_update(dt);
  }

  void on_button(uint32_t buttons) override {
    for (auto *e : entities_)
      e->on_button(buttons);
  }

  void on_joystick(float jx, float jy, float j2x, float j2y) override {
    for (auto *e : entities_)
      e->on_joystick(jx, jy, j2x, j2y);
  }

  void on_trigger(float lt, float rt) override {
    for (auto *e : entities_)
      e->on_trigger(lt, rt);
  }

protected:
  void add_entity(entity *e) {
    entities_.push_back(e);
    e->on_enter();
  }

private:
  std::vector<entity *> entities_;
};

} // namespace dcearth

#endif
