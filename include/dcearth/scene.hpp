#ifndef DCEARTH_SCENE_HPP
#define DCEARTH_SCENE_HPP

#include "dcearth/entity.hpp"
#include <vector>

namespace dcearth {

template <typename T> class scene : public entity<T> {
public:
  void on_exit() override {
    entity<T>::on_exit();
    for (auto *e : entities_)
      e->on_exit();
  }

  void on_render(float dt) override {
    entity<T>::on_render(dt);
    for (auto *e : entities_)
      e->on_render(dt);
  }

  void on_update(float dt) override {
    entity<T>::on_update(dt);
    for (auto *e : entities_)
      e->on_update(dt);
  }

  void on_button(uint32_t buttons) override {
    entity<T>::on_button(buttons);
    for (auto *e : entities_)
      e->on_button(buttons);
  }

  void on_joystick(float jx, float jy, float j2x, float j2y) override {
    entity<T>::on_joystick(jx, jy, j2x, j2y);
    for (auto *e : entities_)
      e->on_joystick(jx, jy, j2x, j2y);
  }

  void on_trigger(float lt, float rt) override {
    entity<T>::on_trigger(lt, rt);
    for (auto *e : entities_)
      e->on_trigger(lt, rt);
  }

protected:
  void add_entity(entity_base *e) {
    entities_.push_back(e);
    e->on_enter();
  }

private:
  std::vector<entity_base *> entities_;
};

} // namespace dcearth

#endif
