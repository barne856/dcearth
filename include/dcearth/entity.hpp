#ifndef DCEARTH_ENTITY_HPP
#define DCEARTH_ENTITY_HPP

#include <cstdint>

namespace dcearth {

class entity {
public:
  virtual ~entity() = default;
  virtual void on_enter() {}
  virtual void on_exit() {}
  virtual void on_render(float dt) {}
  virtual void on_update(float dt) {}
  virtual void on_button(uint32_t buttons) {}
  virtual void on_joystick(float jx, float jy, float j2x, float j2y) {}
  virtual void on_trigger(float lt, float rt) {}
};

} // namespace dcearth

#endif
